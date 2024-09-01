const matPermutation = require('./mat-permutation');
/**
*This function returns the padded matrix with zeros with respect to a given
* target columns number
*@param {Array.<Array.<Number>>} matrix the matrix we need to pad
*@param {Number} columns in our case, the dynamic dimension
*@returns {Array.<Array.<Number>>} padded matrix
*/
module.exports = function (matrix, {columns}) {
	if (columns < matrix[0].length) {
		throw (new TypeError(`Output columns ${columns} is greater than input columns ${matrix[0].length}`));
	}

	return matPermutation({
		matrix,
		outputSize: [matrix.length, columns],
		rowIndexes: new Array(matrix.length).fill(0).map((_, index) => index),
		colIndexes: new Array(matrix[0].length).fill(0).map((_, index) => index),
	});
};
