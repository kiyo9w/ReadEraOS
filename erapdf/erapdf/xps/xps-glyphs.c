#include "mupdf/xps.h"

#include <../../freetype/include/ft2build.h>
#include <../../freetype/include/freetype/freetype.h>
#include <../../freetype/include/freetype/ftadvanc.h>

static inline int ishex(int a)
{
	return (a >= 'A' && a <= 'F') ||
		(a >= 'a' && a <= 'f') ||
		(a >= '0' && a <= '9');
}

static inline int unhex(int a)
{
	if (a >= 'A' && a <= 'F') return a - 'A' + 0xA;
	if (a >= 'a' && a <= 'f') return a - 'a' + 0xA;
	if (a >= '0' && a <= '9') return a - '0';
	return 0;
}

int
xps_count_font_encodings(fz_font *font)
{
	FT_Face face = font->ft_face;
	return face->num_charmaps;
}

void
xps_identify_font_encoding(fz_font *font, int idx, int *pid, int *eid)
{
	FT_Face face = font->ft_face;
	*pid = face->charmaps[idx]->platform_id;
	*eid = face->charmaps[idx]->encoding_id;
}

void
xps_select_font_encoding(fz_font *font, int idx)
{
	FT_Face face = font->ft_face;
	FT_Set_Charmap(face, face->charmaps[idx]);
}

int
xps_encode_font_char(fz_font *font, int code)
{
	FT_Face face = font->ft_face;
	int gid = FT_Get_Char_Index(face, code);
	if (gid == 0 && face->charmap && face->charmap->platform_id == 3 && face->charmap->encoding_id == 0)
		gid = FT_Get_Char_Index(face, 0xF000 | code);
	return gid;
}

void
xps_measure_font_glyph(fz_context *ctx, xps_document *doc, fz_font *font, int gid, xps_glyph_metrics *mtx)
{
	int mask = FT_LOAD_NO_SCALE | FT_LOAD_IGNORE_TRANSFORM;
	FT_Face face = font->ft_face;
	FT_Fixed hadv = 0, vadv = 0;

	fz_lock(ctx, FZ_LOCK_FREETYPE);
	FT_Get_Advance(face, gid, mask, &hadv);
	FT_Get_Advance(face, gid, mask | FT_LOAD_VERTICAL_LAYOUT, &vadv);
	fz_unlock(ctx, FZ_LOCK_FREETYPE);

	mtx->hadv = hadv / (float)face->units_per_EM;
	mtx->vadv = vadv / (float)face->units_per_EM;
	mtx->vorg = face->ascender / (float) face->units_per_EM;
}

static fz_font *
xps_lookup_font(fz_context *ctx, xps_document *doc, char *name)
{
	xps_font_cache *cache;
	for (cache = doc->font_table; cache; cache = cache->next)
		if (!xps_strcasecmp(cache->name, name))
			return fz_keep_font(ctx, cache->font);
	return NULL;
}

static void
xps_insert_font(fz_context *ctx, xps_document *doc, char *name, fz_font *font)
{
	xps_font_cache *cache = fz_malloc_struct(ctx, xps_font_cache);
	cache->name = fz_strdup(ctx, name);
	cache->font = fz_keep_font(ctx, font);
	cache->next = doc->font_table;
	doc->font_table = cache;
}

/*
 * Some fonts in XPS are obfuscated by XOR:ing the first 32 bytes of the
 * data with the GUID in the fontname.
 */
static void
xps_deobfuscate_font_resource(fz_context *ctx, xps_document *doc, xps_part *part)
{
	unsigned char buf[33];
	unsigned char key[16];
	char *p;
	int i;

	if (part->size < 32)
	{
		fz_warn(ctx, "insufficient data for font deobfuscation");
		return;
	}

	p = strrchr(part->name, '/');
	if (!p)
		p = part->name;

	for (i = 0; i < 32 && *p; p++)
	{
		if (ishex(*p))
			buf[i++] = *p;
	}
	buf[i] = 0;

	if (i != 32)
	{
		fz_warn(ctx, "cannot extract GUID from obfuscated font part name");
		return;
	}

	for (i = 0; i < 16; i++)
		key[i] = unhex(buf[i*2+0]) * 16 + unhex(buf[i*2+1]);

	for (i = 0; i < 16; i++)
	{
		part->data[i] ^= key[15-i];
		part->data[i+16] ^= key[15-i];
	}
}

static void
xps_select_best_font_encoding(fz_context *ctx, xps_document *doc, fz_font *font)
{
	static struct { int pid, eid; } xps_cmap_list[] =
	{
		{ 3, 10 },		/* Unicode with surrogates */
		{ 3, 1 },		/* Unicode without surrogates */
		{ 3, 5 },		/* Wansung */
		{ 3, 4 },		/* Big5 */
		{ 3, 3 },		/* Prc */
		{ 3, 2 },		/* ShiftJis */
		{ 3, 0 },		/* Symbol */
		{ 1, 0 },
		{ -1, -1 },
	};

	int i, k, n, pid, eid;

	n = xps_count_font_encodings(font);
	for (k = 0; xps_cmap_list[k].pid != -1; k++)
	{
		for (i = 0; i < n; i++)
		{
			xps_identify_font_encoding(font, i, &pid, &eid);
			if (pid == xps_cmap_list[k].pid && eid == xps_cmap_list[k].eid)
			{
				xps_select_font_encoding(font, i);
				return;
			}
		}
	}

	fz_warn(ctx, "cannot find a suitable cmap");
}

/*
 * Parse and draw an XPS <Glyphs> element.
 *
 * Indices syntax:

 GlyphIndices	= GlyphMapping ( ";" GlyphMapping )
 GlyphMapping	= ( [ClusterMapping] GlyphIndex ) [GlyphMetrics]
 ClusterMapping = "(" ClusterCodeUnitCount [":" ClusterGlyphCount] ")"
 ClusterCodeUnitCount	= * DIGIT
 ClusterGlyphCount		= * DIGIT
 GlyphIndex		= * DIGIT
 GlyphMetrics	= "," AdvanceWidth ["," uOffset ["," vOffset]]
 AdvanceWidth	= ["+"] RealNum
 uOffset		= ["+" | "-"] RealNum
 vOffset		= ["+" | "-"] RealNum
 RealNum		= ((DIGIT ["." DIGIT]) | ("." DIGIT)) [Exponent]
 Exponent		= ( ("E"|"e") ("+"|"-") DIGIT )

 */

static char *
xps_parse_digits(char *s, int *digit)
{
	*digit = 0;
	while (*s >= '0' && *s <= '9')
	{
		*digit = *digit * 10 + (*s - '0');
		s ++;
	}
	return s;
}

static inline int is_real_num_char(int c)
{
	return (c >= '0' && c <= '9') ||
		c == 'e' || c == 'E' || c == '+' || c == '-' || c == '.';
}

static char *
xps_parse_real_num(char *s, float *number)
{
	char buf[64];
	char *p = buf;
	while (is_real_num_char(*s))
		*p++ = *s++;
	*p = 0;
	if (buf[0])
		*number = fz_atof(buf);
	return s;
}

static char *
xps_parse_cluster_mapping(char *s, int *code_count, int *glyph_count)
{
	if (*s == '(')
		s = xps_parse_digits(s + 1, code_count);
	if (*s == ':')
		s = xps_parse_digits(s + 1, glyph_count);
	if (*s == ')')
		s ++;
	return s;
}

static char *
xps_parse_glyph_index(char *s, int *glyph_index)
{
	if (*s >= '0' && *s <= '9')
		s = xps_parse_digits(s, glyph_index);
	return s;
}

static char *
xps_parse_glyph_metrics(char *s, float *advance, float *uofs, float *vofs)
{
	if (*s == ',')
		s = xps_parse_real_num(s + 1, advance);
	if (*s == ',')
		s = xps_parse_real_num(s + 1, uofs);
	if (*s == ',')
		s = xps_parse_real_num(s + 1, vofs);
	return s;
}

/*
 * Parse unicode and indices strings and encode glyphs.
 * Calculate metrics for positioning.
 */
static fz_text *
xps_parse_glyphs_imp(fz_context *ctx, xps_document *doc, const fz_matrix *ctm,
	fz_font *font, float size, float originx, float originy,
	int is_sideways, int bidi_level,
	char *indices, char *unicode)
{
	xps_glyph_metrics mtx;
	fz_text *text;
	fz_matrix tm;
	float e, f;
	float x = originx;
	float y = originy;
	char *us = unicode;
	char *is = indices;
	int un = 0;

	if (!unicode && !indices)
		fz_warn(ctx, "glyphs element with neither characters nor indices");

	if (us)
	{
		if (us[0] == '{' && us[1] == '}')
			us = us + 2;
		un = strlen(us);
	}

	if (is_sideways)
	{
		fz_pre_scale(fz_rotate(&tm, 90), -size, size);
	}
	else
		fz_scale(&tm, size, -size);

	text = fz_new_text(ctx, font, &tm, is_sideways);

	while ((us && un > 0) || (is && *is))
	{
		int char_code = '?';
		int code_count = 1;
		int glyph_count = 1;

		if (is && *is)
		{
			is = xps_parse_cluster_mapping(is, &code_count, &glyph_count);
		}

		if (code_count < 1)
			code_count = 1;
		if (glyph_count < 1)
			glyph_count = 1;

		/* TODO: add code chars with cluster mappings for text extraction */

		while (code_count--)
		{
			if (us && un > 0)
			{
				int t = fz_chartorune(&char_code, us);
				us += t; un -= t;
			}
		}

		while (glyph_count--)
		{
			int glyph_index = -1;
			float u_offset = 0;
			float v_offset = 0;
			float advance;

			if (is && *is)
				is = xps_parse_glyph_index(is, &glyph_index);

			if (glyph_index == -1)
				glyph_index = xps_encode_font_char(font, char_code);

			xps_measure_font_glyph(ctx, doc, font, glyph_index, &mtx);
			if (is_sideways)
				advance = mtx.vadv * 100;
			else if (bidi_level & 1)
				advance = -mtx.hadv * 100;
			else
				advance = mtx.hadv * 100;

			if (font->ft_bold)
				advance *= 1.02f;

			if (is && *is)
			{
				is = xps_parse_glyph_metrics(is, &advance, &u_offset, &v_offset);
				if (*is == ';')
					is ++;
			}

			if (bidi_level & 1)
				u_offset = -mtx.hadv * 100 - u_offset;

			u_offset = u_offset * 0.01f * size;
			v_offset = v_offset * 0.01f * size;

			if (is_sideways)
			{
				e = x + u_offset + (mtx.vorg * size);
				f = y - v_offset + (mtx.hadv * 0.5f * size);
			}
			else
			{
				e = x + u_offset;
				f = y - v_offset;
			}

			fz_add_text(ctx, text, glyph_index, char_code, e, f);

			x += advance * 0.01f * size;
		}
	}

	return text;
}

void
xps_parse_glyphs(fz_context *ctx, xps_document *doc, const fz_matrix *ctm,
		char *base_uri, xps_resource *dict, fz_xml *root)
{
	fz_device *dev = doc->dev;

	fz_xml *node;

	char *fill_uri;
	char *opacity_mask_uri;

	char *bidi_level_att;
	char *fill_att;
	char *font_size_att;
	char *font_uri_att;
	char *origin_x_att;
	char *origin_y_att;
	char *is_sideways_att;
	char *indices_att;
	char *unicode_att;
	char *style_att;
	char *transform_att;
	char *clip_att;
	char *opacity_att;
	char *opacity_mask_att;
	char *navigate_uri_att;

	fz_xml *transform_tag = NULL;
	fz_xml *clip_tag = NULL;
	fz_xml *fill_tag = NULL;
	fz_xml *opacity_mask_tag = NULL;

	char *fill_opacity_att = NULL;

	xps_part *part;
	fz_font *font;

	char partname[1024];
	char fakename[1024];
	char *subfont;

	float font_size = 10;
	int subfontid = 0;
	int is_sideways = 0;
	int bidi_level = 0;

	fz_text *text;
	fz_rect area;

	fz_matrix local_ctm = *ctm;

	/*
	 * Extract attributes and extended attributes.
	 */

	bidi_level_att = fz_xml_att(root, "BidiLevel");
	fill_att = fz_xml_att(root, "Fill");
	font_size_att = fz_xml_att(root, "FontRenderingEmSize");
	font_uri_att = fz_xml_att(root, "FontUri");
	origin_x_att = fz_xml_att(root, "OriginX");
	origin_y_att = fz_xml_att(root, "OriginY");
	is_sideways_att = fz_xml_att(root, "IsSideways");
	indices_att = fz_xml_att(root, "Indices");
	unicode_att = fz_xml_att(root, "UnicodeString");
	style_att = fz_xml_att(root, "StyleSimulations");
	transform_att = fz_xml_att(root, "RenderTransform");
	clip_att = fz_xml_att(root, "Clip");
	opacity_att = fz_xml_att(root, "Opacity");
	opacity_mask_att = fz_xml_att(root, "OpacityMask");
	navigate_uri_att = fz_xml_att(root, "FixedPage.NavigateUri");

	for (node = fz_xml_down(root); node; node = fz_xml_next(node))
	{
		if (fz_xml_is_tag(node, "Glyphs.RenderTransform"))
			transform_tag = fz_xml_down(node);
		if (fz_xml_is_tag(node, "Glyphs.OpacityMask"))
			opacity_mask_tag = fz_xml_down(node);
		if (fz_xml_is_tag(node, "Glyphs.Clip"))
			clip_tag = fz_xml_down(node);
		if (fz_xml_is_tag(node, "Glyphs.Fill"))
			fill_tag = fz_xml_down(node);
	}

	fill_uri = base_uri;
	opacity_mask_uri = base_uri;

	xps_resolve_resource_reference(ctx, doc, dict, &transform_att, &transform_tag, NULL);
	xps_resolve_resource_reference(ctx, doc, dict, &clip_att, &clip_tag, NULL);
	xps_resolve_resource_reference(ctx, doc, dict, &fill_att, &fill_tag, &fill_uri);
	xps_resolve_resource_reference(ctx, doc, dict, &opacity_mask_att, &opacity_mask_tag, &opacity_mask_uri);

	/*
	 * Check that we have all the necessary information.
	 */

	if (!font_size_att || !font_uri_att || !origin_x_att || !origin_y_att) {
		fz_warn(ctx, "missing attributes in glyphs element");
		return;
	}

	if (!indices_att && !unicode_att)
		return; /* nothing to draw */

	if (is_sideways_att)
		is_sideways = !strcmp(is_sideways_att, "true");

	if (bidi_level_att)
		bidi_level = atoi(bidi_level_att);

	/*
	 * Find and load the font resource
	 */

	xps_resolve_url(ctx, doc, partname, base_uri, font_uri_att, sizeof partname);
	subfont = strrchr(partname, '#');
	if (subfont)
	{
		subfontid = atoi(subfont + 1);
		*subfont = 0;
	}

	/* Make a new part name for font with style simulation applied */
	fz_strlcpy(fakename, partname, sizeof fakename);
	if (style_att)
	{
		if (!strcmp(style_att, "BoldSimulation"))
			fz_strlcat(fakename, "#Bold", sizeof fakename);
		else if (!strcmp(style_att, "ItalicSimulation"))
			fz_strlcat(fakename, "#Italic", sizeof fakename);
		else if (!strcmp(style_att, "BoldItalicSimulation"))
			fz_strlcat(fakename, "#BoldItalic", sizeof fakename);
	}

	font = xps_lookup_font(ctx, doc, fakename);
	if (!font)
	{
		fz_buffer *buf = NULL;
		fz_var(buf);

		fz_try(ctx)
		{
			part = xps_read_part(ctx, doc, partname);
		}
		fz_catch(ctx)
		{
			fz_rethrow_if(ctx, FZ_ERROR_TRYLATER);
			fz_warn(ctx, "cannot find font resource part '%s'", partname);
			return;
		}

		/* deobfuscate if necessary */
		if (strstr(part->name, ".odttf"))
			xps_deobfuscate_font_resource(ctx, doc, part);
		if (strstr(part->name, ".ODTTF"))
			xps_deobfuscate_font_resource(ctx, doc, part);

		fz_try(ctx)
		{
			buf = fz_new_buffer_from_data(ctx, part->data, part->size);
			/* part->data is now owned by buf */
			part->data = NULL;
			font = fz_new_font_from_buffer(ctx, NULL, buf, subfontid, 1);
		}
		fz_always(ctx)
		{
			fz_drop_buffer(ctx, buf);
			xps_drop_part(ctx, doc, part);
		}
		fz_catch(ctx)
		{
			fz_rethrow_if(ctx, FZ_ERROR_TRYLATER);
			fz_warn(ctx, "cannot load font resource '%s'", partname);
			return;
		}

		if (style_att)
		{
			font->ft_bold = !!strstr(style_att, "Bold");
			font->ft_italic = !!strstr(style_att, "Italic");
		}

		xps_select_best_font_encoding(ctx, doc, font);
		xps_insert_font(ctx, doc, fakename, font);
	}

	/*
	 * Set up graphics state.
	 */

	if (transform_att || transform_tag)
	{
		fz_matrix transform;
		if (transform_att)
			xps_parse_render_transform(ctx, doc, transform_att, &transform);
		if (transform_tag)
			xps_parse_matrix_transform(ctx, doc, transform_tag, &transform);
		fz_concat(&local_ctm, &transform, &local_ctm);
	}

	if (clip_att || clip_tag)
		xps_clip(ctx, doc, &local_ctm, dict, clip_att, clip_tag);

	font_size = fz_atof(font_size_att);

	text = xps_parse_glyphs_imp(ctx, doc, &local_ctm, font, font_size,
			fz_atof(origin_x_att), fz_atof(origin_y_att),
			is_sideways, bidi_level, indices_att, unicode_att);

	fz_bound_text(ctx, text, NULL, &local_ctm, &area);

	if (navigate_uri_att)
		xps_add_link(ctx, doc, &area, base_uri, navigate_uri_att);

	xps_begin_opacity(ctx, doc, &local_ctm, &area, opacity_mask_uri, dict, opacity_att, opacity_mask_tag);

	/* If it's a solid color brush fill/stroke do a simple fill */

	if (fill_tag && !strcmp(fz_xml_tag(fill_tag), "SolidColorBrush"))
	{
		fill_opacity_att = fz_xml_att(fill_tag, "Opacity");
		fill_att = fz_xml_att(fill_tag, "Color");
		fill_tag = NULL;
	}

	if (fill_att)
	{
		float samples[FZ_MAX_COLORS];
		fz_colorspace *colorspace;

		xps_parse_color(ctx, doc, base_uri, fill_att, &colorspace, samples);
		if (fill_opacity_att)
			samples[0] *= fz_atof(fill_opacity_att);
		xps_set_color(ctx, doc, colorspace, samples);

		fz_fill_text(ctx, dev, text, &local_ctm,
			doc->colorspace, doc->color, doc->alpha);
	}

	/* If it's a complex brush, use the charpath as a clip mask */

	if (fill_tag)
	{
		fz_clip_text(ctx, dev, text, &local_ctm, 0);
		xps_parse_brush(ctx, doc, &local_ctm, &area, fill_uri, dict, fill_tag);
		fz_pop_clip(ctx, dev);
	}

	xps_end_opacity(ctx, doc, opacity_mask_uri, dict, opacity_att, opacity_mask_tag);

	fz_drop_text(ctx, text);

	if (clip_att || clip_tag)
		fz_pop_clip(ctx, dev);

	fz_drop_font(ctx, font);
}
