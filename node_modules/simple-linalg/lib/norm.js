module.exports = function (vector) {
	let result = 0;
	for (const element of vector) {
		result += (element * element);
	}

	return Math.sqrt(result);
};
