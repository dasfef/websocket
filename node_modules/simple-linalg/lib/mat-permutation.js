/**
 *
 * @param {Array.<Array.<Number>>} matrix
 * @param {[Number, Number]} outputSize
 * @param {Array.<Number>} rowIndexes the permutation indexes, result[j][k] = matrix[rowIndexes.indexOf(j)][colIndexes.indexOf(k)]
 * @param {Array.<Number>} colIndexes the permutation indexes, result[j][k] = matrix[rowIndexes.indexOf(j)][colIndexes.indexOf(k)]
 * @returns {Array.<Array.<Number>>}
 */
module.exports = function ({
	matrix,
	outputSize,
	rowIndexes,
	colIndexes,
}) {
	const [nRow, nCol] = outputSize;

	if (!Array.isArray(rowIndexes)) {
		throw (new TypeError(`Invalid rowIndexes ${rowIndexes}`));
	}

	if (!Array.isArray(colIndexes)) {
		throw (new TypeError(`Invalid colIndexes ${colIndexes}`));
	}

	return new Array(nRow).fill(0).map((_, i) => new Array(nCol).fill(0).map((_, j) => {
		if (colIndexes.includes(j) && rowIndexes.includes(i)) {
			return matrix[rowIndexes.indexOf(i)][colIndexes.indexOf(j)];
		}

		return 0;
	}));
};
