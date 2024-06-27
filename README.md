### Генератор Си кода из описанного формата блок-схем

#### Dependencies:
- Boost v1.82
- (pugixml v1.14 [ref](https://github.com/zeux/pugixml) - compiled as third-party lib)
- CMake v3.28
- Doxygen v1.9

#### Build:
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

#### Usage:
Help:
```
./translator -h
```

Default source:
```
./translator
```

Choose source:
```
./translator -s source.xml
```

#### Concept:
<!-- here must be IR description -->

#### Docs:
- https://notzahar.github.io/translator/
