declare module 'simple-linalg' {
	export function add(...matrices: number[][][]): number[][];
	export function cosSimilarity(vector1: number[], vector2: number[]): number;
	export function diag(diagonal: number[]): number[][];
	export function diagBlock(options: {blocks: number[][][]; order: number[]}): number[][];
	export function euclideanDist(vector1: number[], vector2: number[]): number;
	export function frobenius(a: number[][], b: number[][]): number;
	// eslint-disable-next-line unicorn/prevent-abbreviations
	export function elemWise(matrices: number[][][], callback: (list: number[], rowId: number, colId: number) => void): void;
	export function identity(n: number): number[][];
	export function invert(matrix: number[][]): number[][];
	export function matMul(a: number[][], b: number[][]): number[][];
	export function matPermutation(options: {matrix: number[][]; outputSize: number[]; colIndexes: number[]; rowIndexes: number[]}): number[][];
	export function padWithZeroCols(matrix: number[][], options: {columns: number}): number[][];
	export function subtract(a: number[][], b: number[][]): number[][];
	export function subSquareMatrix(matrix: number[][], indices: number[]): number[][];
	export function sum(matrix: number[][]): number;
	export function trace(matrix: number[][]): number;
	export function transpose(matrix: number[][]): number[][];
	export function zeros(n: number): number[][];
	export function norm(vector: number[]): number;
	export function mapMatrix(matrix: number[][], callback: (value: number, rowId: number, colId: number) => void): void;
	export function dotProduct(a: number[], b: number[]): number;
}
