Базовое приложение glfw+glad+glm+ImGui с применением шейдеров. Построение кривой по точкам


//Сборка под Windows
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release .. -DCMAKE_CXX_FLAGS="-mwindows"
cmake --build . -j4

//Сборка под Linux Manjaro
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j16
или
ninja -j40
