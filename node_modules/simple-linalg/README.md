# simple-linalg 😄

Simple Linear algebra function in JS

## Installation 📦

```bash
npm install simple-linalg
```

## Motivation 🚀

`simple-linalg` is a spin-off project from the [Kalman Filter](https://github.com/piercus/kalman-filter) library. We created this library to provide essential linear algebra functions in a user-friendly, efficient, and accessible way. 

We believe in keeping things simple and efficient, and that's what `simple-linalg` is all about!

Here's what makes simple-linalg stand out:

*KISS*: Keep It Simple and Stupid 😇 - We focus on delivering a straightforward and easy-to-understand library.
*Pure Functional*: All functions are designed to be pure and functional, which makes it easier to reason about the code 📚.
*Native JS Array Manipulation*: We use native JavaScript arrays for simplicity and compatibility with a wide range of projects 🌐.
*Small Footprint*: Our only dependency is matrix-inverse, which keeps the library lightweight and easy to integrate 🎈.

So, if you're looking for a simple, efficient, and easy-to-use linear algebra library for your JavaScript projects, look no further! `simple-linalg` is here to help you out! 😊

## Examples

### add

```js
const {add} = require('simple-linalg');
const a = [[3, 7], [4, 9]];
const b = [[6, 2], [5, 8]];

add(a, b) 
// => [[9, 9], [9, 17]];

```

### diag

```js
const {diag} = require('simple-linalg');

diag([3, 7]) 
// => [[3, 0], [7, 0]];
```

### diagBlock

```js
const {diagBlock} = require('simple-linalg');
const a = [[3, 7], [4, 9]];
const b = [[6, 2], [5, 8]];

const blocks = [a, b];

const result = diagBlock({blocks});

// => [[3, 7, 0, 0], [4, 9, 0, 0], [0, 0, 6, 2], [0, 0, 5, 8]];
```

### cosSimilarity

```js
const {cosSimilarity} = require('simple-linalg');

const a = [0, 7];
const b = [0, 2];

cosSimilarity(a, b) 
// => 1
```

### dotProduct

```js
const {dotProduct} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
const b = [[6, 2], [5, 8]];

dotProduct(a, b) 
// => [[18, 14], [20, 72]];
```


### euclideanDist

```js
const {euclideanDist} = require('simple-linalg');

const a = [3, 6];
const b = [6, 2];

euclideanDist(a, b) 
// => 5
```


### elemWise

```js
const {elemWise} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
const b = [[6, 2], [5, 8]];

let sum = 0;
elemWise([a,b], function(list, rowId, colId){
	const [aValue, bValue] = list;
	sum += aValue+bValue;
	// you can use rowId and colId here
}) 

// sum is 44
```


### frobenius

```js
const {frobenius} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
const b = [[6, 2], [5, 8]];

frobenius(a, b) 
// => 6
```


### identity

```js
const {identity} = require('simple-linalg');

identity(2) 

// [[1,0],[0,1]]
```

### identity

```js
const {invert} = require('simple-linalg');

identity(2) 

// [[1,0],[0,1]]
```

### invert

```js
const {invert} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
invert(a) 
```

### matMul
```js
const {matMul} = require('simple-linalg');
const a = [[3, 7], [4, 9]];
const b = [[6, 2], [5, 8]];

matMul(a, b) 
// => [[53, 62], [69, 80]];

```


### matPermutation
```js
const {matPermutation} = require('simple-linalg');
const a = [[3, 7], [4, 9]];

matPermutation({matrix: a, outputSize: [3,3], rowIndexes: [2, 0], colIndexes: [0,2]}) 
// => [[4, 0, 9], [0, 0, 0], [3, 0, 7]];

```

### mapMatrix

```js
const {mapMatrix} = require('simple-linalg');
const a = [[3, 7], [4, 9]];


let sum = 0;
mapMatrix([a,b], function(value, rowId, colId){
	sum += value;
	// you can use rowId and colId here
}) 
// => sum is 23;
```

### norm

```js
const {norm} = require('simple-linalg');
const a = [3, 4];

norm(a)
// => norm is 5;
```

### padWithZeroCols

```js
const {padWithZeroCols} = require('simple-linalg');
const a = [[3, 7], [4, 9]];

padWithZeroCols(a, {columns: 4}) 
// => [[3, 7, 0, 0], [4, 9, 0, 0]];

```

### subtract

```js
const {subtract} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
const b = [[6, 2], [5, 8]];

sub(a, b) 
// => [[-3, 5], [-1, 1]];

```

### subSquareMatrix

```js
const {subSquareMatrix} = require('simple-linalg');

const a = [[3, 7, 4, 5], [4, 9, 8, 7], [8, 7, 2, -1]];

subSquareMatrix(a, [0, 2]) 
// => [[3, 4], [8, 2]];
```

### sum

```js
const {sum} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
sum(a)

// => 23;
```


### trace

```js
const {trace} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
trace(a)

// => 7;
```

### transpose

```js
const {transpose} = require('simple-linalg');

const a = [[3, 7], [4, 9]];
transpose(a)

// => [[3, 9], [4, 7]];
```

### zeros

```js
const {zeros} = require('simple-linalg');

zeros(2)

// => [[0, 0], [0, 0]];
```

### Going further

This library is not optimized for performance or complex usage
If you want to focus on performance, I would suggest moving to Native C-bindings.
For example [opencv4nodejs](https://github.com/UrielCh/opencv4nodejs)