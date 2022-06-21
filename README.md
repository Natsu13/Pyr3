# Pyr3
New programming language in C++

Some example
Factorial!

Calculated in 0.005 ms

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

```
number:s64 = 13;
if typeof(number) == s16 {
    print("number is s16");
}else{
    print("number = ");
    print(typeof(number));
}
```

```
start :: 0;
for index, key: start..start + 10 {
    print(key);
    print(": ");
    print(index);
    print("\n");
}
```

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

```
GetForegroundWindow :: () -> ptr #foreign "USER32.DLL";
```

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

```
main :: () {
    arr: Array;
    arr.count = 2;
    arr.data = malloc(8 * 2);
    arr.data[0] = 50;
    arr.data[1] = 100;
    print(arr.count);
    print(arr.data[0]);
    print(arr.data[1]);
}

Array :: struct {
    count: s64;
    data: *s64;
}
```
