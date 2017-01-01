stanza_geom_all: ${GEN}/ao.pkg ${GEN}/ao-gfx.pkg ${GEN}/ao-eval.pkg ${GEN}/geom.pkg ${GEN}/geom-gfx.pkg 

ALL_PKG_DEPS += stanza_geom_all

${GEN}/ao.pkg: stanza-geom/ao.stanza ${GEN}/utils.pkg ${GEN}/geom.pkg ${GEN}/font.pkg ${GEN}/glu.pkg
	stanza $< $(STZ_FLAGS)

${GEN}/ao-gfx.pkg: stanza-geom/ao-gfx.stanza ${GEN}/gl.pkg ${GEN}/geom.pkg ${GEN}/geom-gfx.pkg ${GEN}/ao.pkg
	stanza $< $(STZ_FLAGS)

${GEN}/ao-eval.pkg: stanza-geom/ao-eval.stanza ${GEN}/utils.pkg ${GEN}/eval.pkg ${GEN}/geom.pkg ${GEN}/glu.pkg ${GEN}/ao.pkg ${GEN}/font.pkg
	stanza $< $(STZ_FLAGS)

${GEN}/geom.pkg: stanza-geom/geom.stanza ${GEN}/utils.pkg 
	stanza $< $(STZ_FLAGS)

${GEN}/geom-gfx.pkg: stanza-geom/geom-gfx.stanza ${GEN}/geom.pkg ${GEN}/gfx.pkg ${GEN}/font.pkg
	stanza $< $(STZ_FLAGS)

