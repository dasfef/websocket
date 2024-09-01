const dotProduct = require('./dot-product.js');
const norm = require('./norm.js');

module.exports = function (vector1, vector2) {
	if (vector1.length !== vector2.length) {
		throw (new Error('The lengths of the vectors do not match'));
	}

	const normProd = (norm(vector1) * norm(vector2));

	if (normProd === 0) {
		return 0;
	}

	return dotProduct(vector1, vector2) / normProd;
};
