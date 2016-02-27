function [ ] = ShowLabeledImage( img )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goesUntitled2 here
    tmp = double(img);
    img_min = min(min(tmp));
    img_max = max(max(tmp));
    tmp = (tmp(:,:) - img_min) / (img_max - img_min);
    figure; imshow(tmp,'InitialMagnification','fit');
    colormap(jet);
end

