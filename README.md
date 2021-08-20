# Pyr3
New programming language in C++

Some example
Factorial!

Calculated in 0.005 ms

```
main :: () {
	fact := factorial(20);
}

factorial :: (x: u64) -> u64 {
	if x > 1 {
		return x * factorial(x - 1);
	}

	return 1;
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
