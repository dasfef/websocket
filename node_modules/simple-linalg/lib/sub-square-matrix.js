module.exports = (mat, indexes) => {
	return indexes.map(s1 => indexes.map(s2 => mat[s1][s2]));
};
