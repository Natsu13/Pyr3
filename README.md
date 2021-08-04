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

GetForegroundWindow :: () -> ptr #foreign "USER32.DLL";
```
