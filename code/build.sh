pushd ../build/

INCLUDE="-I../code/include/raylib -I../code/include/rlImGui -I../code/include/imgui"
LIBS="-L. libraylib.a librlImGui.a"
SHARED_OBJECTS="./libraylib.so.450 ./imgui.so"

g++ -fno-gnu-unique -rdynamic -shared -fPIC -o misery.so $INCLUDE ../code/misery.cpp -g
g++ -fno-gnu-unique -o misery.x86_64 $INCLUDE $LIBS $SHARED_OBJECTS ../code/misery_linux.cpp -g

popd