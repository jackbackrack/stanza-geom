stanza_geom_all: ${GEN}/geom.pkg ${GEN}/geom-gfx.pkg 

ALL_PKG_DEPS += stanza_geom_all

${GEN}/geom.pkg: stanza-geom/geom.stanza ${GEN}/utils.pkg 
	stanza $< $(STZ_FLAGS)

${GEN}/geom-gfx.pkg: stanza-geom/geom-gfx.stanza ${GEN}/geom.pkg ${GEN}/gfx.pkg ${GEN}/font.pkg
	stanza $< $(STZ_FLAGS)

