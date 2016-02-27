function [ predictImgs ] = Construct_labels_from_prediction( segIds, segmentImages, predictions )
%UNTITLED2 Summary of this function goes here
%   73 is the image, 74 is the segmentation id, 75 is the ground truth
%   Constructs prediction images from a set of 1xN segIds, HxWxN segmentImages and 1xN predictions. 
numSamples = length(predictions);
height = size(segmentImages,1);
width = size(segmentImages,2);
predictImgs = uint8(zeros(height, width, numSamples));
for i=1:numSamples
    predictImgs(segmentImages(:,:,:) == segIds(i)) = predictions(i);
end
end