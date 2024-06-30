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
<!-- here must be U description -->

```
  <Block BlockType="Inport" Name="setpoint" SID="16">
    <P Name="Position">[-20, 403, 10, 417]</P>
    <Port>                                        <!-- Судя по всему эта строчка - обозначение выходного порта -->
      <P Name="PortNumber">1</P>
      <P Name="Name">setpoint</P>                 
    </Port>
  </Block>
  <Block BlockType="Inport" Name="feedback" SID="18">
    <P Name="Position">[-20, 453, 10, 467]</P>
    <P Name="Port">2</P>                          <!-- Непонятно зачем нужна эта строка -->
    <Port>
      <P Name="PortNumber">1</P>
      <P Name="Name">feedback</P>
    </Port>
  </Block>
  <Block BlockType="Sum" Name="Add1" SID="17">
    <P Name="Ports">[2, 1]</P>                    <!-- Непонятно зачем нужна эта строка -->
    <P Name="Position">[105, 402, 135, 433]</P>
    <P Name="IconShape">rectangular</P>
    <P Name="Inputs">+-</P>
  </Block>
```

#### Docs:
- https://notzahar.github.io/translator/
