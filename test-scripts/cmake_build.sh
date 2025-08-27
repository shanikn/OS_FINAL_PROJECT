if [[ -d "build" ]]; then
  rm -rf build
fi
mkdir "build"
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel