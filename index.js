const feature = require('./build/Release/feature');

exports.getImageFeature = (filePath) => {
  return feature.imageFeature(filePath).toString('utf8');
};

exports.getImageSimilarity = (descriptor1, descriptor2) => {
  let matBuffer1 = Buffer.from(descriptor1);
  let matBuffer2 = Buffer.from(descriptor2);
  return feature.similarity(matBuffer1, matBuffer1.length, matBuffer2, matBuffer2.length);
};
