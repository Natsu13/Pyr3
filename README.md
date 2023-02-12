# Pyr3
New programming language in C++

Some examples:

Example of Factorial! Calculated in 0.005 ms

```
main :: () {
    fact := factorial(20);
    print(fact);
    if(fact != 2432902008176640000){
        print("\nThis is not correct answer!");
    }
}

factorial :: (x: u64) -> u64 {
    if x > 1 {
        return x * factorial(x - 1);
    }

    return 1;
}
```

Example of typeof conditions
```
number:s64 = 13;
if typeof(number) == s16 {
    print("number is s16");
}else{
    print("number = ");
    print(typeof(number));
}
```

Example of foor lopp and range operator
```
start :: 0;
for index, key: start..start + 10 {
    print(key);
    print(": ");
    print(index);
    print("\n");
}
```

Example of return list
```
main :: (){
    a, b := swap(1,2);
    print(a);
    print(" -- ");
    print(b);
}

swap :: (a: s64, b: s64) -> s64, s64 {
    return b, a;
}
```

Example of typeof, sizeof
```
main :: () {
    print(typeof(Types));
    print("\n");
    print(sizeof(string) + sizeof(Types));
    print("\n");
    print(sizeof(TypeDefinition));
}

Types :: enum {
    NUMBER;
    STRING;
    FLOAT;
    BOOL;
}

TypeDefinition :: struct {
    name: string;
    type: Types;
}
```

Example of foreign procedures from system library
```
GetForegroundWindow :: () -> ptr #foreign "USER32.DLL";
```

Example of custom array with pointer index skip
```
arr: *s64;
arr= malloc(128);
arr[0] = 50;
arr[1] = 100;
arr[2] = 200;
print(arr[0]);
print(arr[1]);
print(arr[2]);
```

Example of build in Array
```
main :: () {
    arr: s64[2];        
    arr[0] = 50;
    arr[1] = 100;
    print(arr.count);
    print(arr[0]);
    print(arr[1]);
}

Array :: struct {
    count: s64;
    data: *s64;
}
```

Example of Union in Struct
```
Test :: struct {
    size: s16;
    x: s64;
    value: union {
        number: s64;
        text: string;        
        x: s64;
        y: s64;
    }
}

main :: () {
    test: Test;
    test.value.x = 36;
    test.value.text = "Hello";
    print(test.value.text);
}
```
