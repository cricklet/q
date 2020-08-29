
INCLUDE_REACTPHYSICS3D = vendor/reactphysics3d/include
LIB_REACTHPHYSICS3D_DIR = vendor/reactphysics3d/build_emcc
LIB_REACTHPHYSICS3D_FILE = vendor/reactphysics3d/build_emcc/libreactphysics3d.a

EMCC_OPTS =  -s WASM=1 --bind -O1 -std=c++17 -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1 -I /usr/local/include -I $(INCLUDE_REACTPHYSICS3D) -g
DEPENDENCY_OPTS = -MMD -MP

TSC_OPTS = --strictNullChecks --noImplicitAny

CPP_FILES := $(wildcard src/cpp/*.cpp)
OBJ_FILES := $(addprefix output/, $(notdir $(CPP_FILES:.cpp=.bc)))
DEPENDS := $(patsubst %.bc,%.d, $(OBJ_FILES))

TS_FILES := $(wildcard src/ts/*ts)

all: data/textures_manifest.json src/cpp/bindings.h output/index.js output/app.js $(LIB_REACTHPHYSICS3D_FILE)

excluding_cpp: data/textures_manifest.json src/cpp/bindings.h output/app.js

src/cpp/bindings.h: generate_bindings.py schema.py
	python3 generate_bindings.py

data/textures_manifest.json: generate_manifests.py schema.py
	python3 generate_manifests.py

output/app.js: $(TS_FILES)
	npx esbuild --bundle src/ts/app.ts --outfile=output/app.js --sourcemap
	tsc $(TSC_OPTS) --noEmit

output/index.js: $(OBJ_FILES) $(LIB_REACTHPHYSICS3D_FILE)
	emcc $(EMCC_OPTS) -o  $@ $(LIB_REACTHPHYSICS3D_FILE) $^ -o output/index.js

-include $(DEPENDS)
output/$(notdir %.bc): src/cpp/%.cpp
	emcc $(EMCC_OPTS) $(DEPENDENCY_OPTS) -c -o $@ $<

$(LIB_REACTHPHYSICS3D_FILE):
	mkdir -p $(LIB_REACTHPHYSICS3D_DIR) \
	&& cd $(LIB_REACTHPHYSICS3D_DIR) \
	&& emcmake ccmake .. \
	&& emmake make \
	&& cd ../../..

# $(LIB_REACTHPHYSICS3D_DIR):
# 	cd src/cpp/reactphysics3d \
# 	&& emcmake ccmake . \
# 	&& emmake make \
# 	&& mv libreactphysics3d.a ../../../$(LIB_REACTHPHYSICS3D_DIR) \
# 	&& cd ../../..

clean:
	@- rm -f output/*
	@- rm -f $(LIB_REACTHPHYSICS3D_DIR)

clean-emcc:
	@- emcc --clear-cache
