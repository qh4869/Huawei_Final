clear all;
close all;
ks4 = load("t1.txt");
% ks6 = load("case1-t6.txt");

figure();
hold on;
plot(ks4(:,1), ks4(:,5), 'r')
% plot(ks6(:,1), ks6(:,5), 'b')
% legend('ks1','ks6')
title('cpu空闲比')

figure();
hold on;
plot(ks4(:,1), ks4(:,6), 'r')
% plot(ks6(:,1), ks6(:,6), 'b')
% legend('ks1','ks6')
title('ram空闲比')

figure();
hold on;
plot(ks4(:,1), ks4(:,7), 'r')
plot(ks4(:,1), ks4(:,8), 'b')
legend('add','del')
title('请求数')

figure();
hold on;
plot(ks4(:,1), ks4(:,3), 'r')
% plot(ks6(:,1), ks6(:,3), 'b')
% legend('ks1','ks6')
title('每天购买硬件成本')

figure();
hold on;
plot(ks4(:,1), ks4(:,2), 'r')
% plot(ks6(:,1), ks6(:,2), 'b')
% legend('ks1','ks6')
title('累计硬件成本')