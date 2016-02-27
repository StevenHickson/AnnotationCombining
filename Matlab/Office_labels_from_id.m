function [ id2Labels ] = Office_labels_from_id( )
%UNTITLED Summary of this function goes here
%   map for label id to label name.
  id2Labels = containers.Map();
  id2Labels(0) = 'background';
  id2Labels(1) = 'books';
  id2Labels(2) = 'cabinets';
  id2Labels(3) = 'ceiling';
  id2Labels(4) = 'chair';
  id2Labels(5) = 'computer';
  id2Labels(6) = 'cup';
  id2Labels(6) = 'bottle';
  id2Labels(7) = 'door';
  id2Labels(8) = 'fire_extinguisher';
  id2Labels(9) = 'floor';
  id2Labels(10) = 'fridge';
  id2Labels(11) = 'keyboard';
  id2Labels(12) = 'monitor';
  id2Labels(13) = 'person';
  id2Labels(14) = 'poster';
  id2Labels(15) = 'signs';
  id2Labels(16) = 'table';
  id2Labels(17) = 'trashcan';
  id2Labels(18) = 'walls';
  id2Labels(19) = 'whiteboard';
end

