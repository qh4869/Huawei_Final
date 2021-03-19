clear all;
close all;
ks1 = load("case1-t1.txt");
ks6 = load("case1-t6.txt");

figure();
hold on;
plot(ks1(:,1), ks1(:,5), 'r')
plot(ks6(:,1), ks6(:,5), 'b')
legend('ks1','ks6')
title('cpu空闲比')

figure();
hold on;
plot(ks1(:,1), ks1(:,6), 'r')
plot(ks6(:,1), ks6(:,6), 'b')
legend('ks1','ks6')
title('ram空闲比')

figure();
hold on;
plot(ks1(:,1), ks1(:,7), 'r')
plot(ks1(:,1), ks1(:,8), 'b')
legend('add','del')
title('请求数')

figure();
hold on;
plot(ks1(:,1), ks1(:,3), 'r')
plot(ks6(:,1), ks6(:,3), 'b')
legend('ks1','ks6')
title('每天购买硬件成本')

figure();
hold on;
plot(ks1(:,1), ks1(:,2), 'r')
plot(ks6(:,1), ks6(:,2), 'b')
legend('ks1','ks6')
title('累计硬件成本')