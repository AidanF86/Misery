pushd ../build/

INCLUDE="-I../code/include/raylib -I../code/include/rlImGui -I../code/include/imgui"
LIBS="-L. libraylib.a librlImGui.a ./libraylib.so.450 ./imgui.so -B./"

g++ -rdynamic -shared -fPIC -o misery.so $INCLUDE $LIBS ../code/misery.cpp -g
g++ -o misery.x86_64 $INCLUDE $LIBS ../code/misery_linux.cpp -g

popd