clear all;
close all;
tr1 = load("reqNum1.txt");
tr2 = load("reqNum2.txt");

figure();
hold on;
plot(tr1(:,1), 'r')
plot(tr1(:,2), 'b')
legend('add','del')
title('tr1请求数')

figure();
hold on;
plot(tr2(:,1), 'r')
plot(tr2(:,2), 'b')
legend('add','del')
title('tr2请求数')