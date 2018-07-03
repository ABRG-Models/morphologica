clear all;
close all;
cortex = csvread('cortex.csv');
a1 = csvread('a1.csv');
v1 = csvread('v1.csv');
s1 = csvread('s1.csv');

plot (cortex(:,1),-cortex(:,2), 'k');
hold on;
plot (a1(:,1),-a1(:,2), 'r');
plot (v1(:,1),-v1(:,2), 'b');
plot (s1(:,1),-s1(:,2), 'g');
