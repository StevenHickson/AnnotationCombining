function [ conf, conf2 ] = PixelAcc( guess, label, method )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
num_classes = max(max(max(label)));
crop = num_classes + 1;
conf = zeros(num_classes,num_classes);
len = length(guess);
count = 0;
nums = zeros(num_classes,1);
dens = zeros(num_classes,1);
myOrder = zeros(crop,1);
for i=1:crop
    myOrder(i) = i - 1;
end
for i=1:len
   if(~isempty(guess{i}))
       count = count + 1;
       tmp1 = double(label(:,:,i));
       tmp2 = guess{i};
       %tmp = zeros(4,4);
       if(strcmp(method,'jaccard'))
           [tmp3, tmp4] = jaccard_conf(tmp2,tmp1, num_classes);
           nums = nums + tmp3;
           dens = dens + tmp4;
       else
           tmp1 = tmp1(:)';
           tmp2 = tmp2(:)';
           tmp = confusionmat(tmp1,tmp2,'order',myOrder);
           if(length(tmp) == crop)
               tmp = tmp(2:crop,2:crop);
           elseif(length(tmp) ~= num_classes)
               %need to figure this out
               tmp = tmp;
           end
           conf = conf + tmp;
       end
   end
end
if(strcmp(method,'jaccard'))
    %conf = conf / count;
    for i=1:num_classes
        conf(i,i) = nums(i) / dens(i);
    end
end

conf2 = conf;
[len, ~] = size(conf2);
sums = sum(conf2');
for i=1:len
    for j=1:len
        conf2(i,j) = conf2(i,j) / sums(i);
    end
end

end

