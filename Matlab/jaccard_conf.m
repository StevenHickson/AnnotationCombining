function [ nums, dens ] = jaccard_conf( guess, label, num_classes )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here 
 %conf = zeros(4,4);
 nums = zeros(num_classes,1);
 dens = zeros(num_classes,1);
 for i=1:num_classes
        log1 = guess;
        log2 = label;
        log1(log1(:,:) ~= i) = 0;
        log1(log1(:,:) == i) = 1;
        log1 = logical(log1);
        log2(log2(:,:) ~= i) = 0;
        log2(log2(:,:) == i) = 1;
        log2 = logical(log2);
        [nums(i), dens(i)] = jaccard_coefficient(log1,log2);
%         if(isnan(dist))
%             conf(i,i) = 1;
%         else
%             conf(i,i) = dist;
%         end
 end
 
end

