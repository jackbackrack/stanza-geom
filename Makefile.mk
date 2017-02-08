stanza_geom_all: ${GEN}/geom.pkg ${GEN}/geom-gfx.pkg 

ALL_PKG_DEPS += stanza_geom_all

${GEN}/geom.pkg: stanza-geom/geom.stanza ${GEN}/utils.pkg 
	stanza $< $(STZ_FLAGS)

${GEN}/eval-geom.stanza: ${GEN}/gen-repl ${GEN}/geom.pkg
	${GEN}/gen-repl geom

${GEN}/eval-geom.pkg: ${GEN}/eval-geom.stanza ${BASE_EVAL_PKGS} ${GEN}/geom.pkg
	stanza $< $(STZ_FLAGS)

${GEN}/geom-gfx.pkg: stanza-geom/geom-gfx.stanza ${GEN}/geom.pkg ${GEN}/gfx.pkg ${GEN}/font.pkg
	stanza $< $(STZ_FLAGS)

