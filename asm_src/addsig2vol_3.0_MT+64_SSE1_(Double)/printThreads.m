
count=10;
senderPos = 0.01.*rand(3,count);
receiverPos = 0.01.*rand(3,count);
IMAGE_STARTPOINT = [0,0,0];
IMAGE_RESOLUTION= 0.001;
Speed=1500;
TimeInterval=1e-7;
DataLength=1000;
Data=floor(rand(DataLength,count).*2);
x=120;


addsig2vol_3_mex(4);
tic;
[n1,n2,n3,n4,n5]=addsig2vol_3_mex(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),zeros([x,x,x]));
t1 = toc



lastTask = max(n4(1,:));
% Plotte Reihenfolge der tasks 
figure;
plot(n3(1:count,:), n4(1:count,:), 'x'); hold on;
xlabel('time[ms]');
ylabel('tasknumber[nacheinanderliegend im Speicher]');

% Trennlinie pro AScan zeichnen
for i=1:1:count
  plot([n3(i,1),n3(i,1)], [0,lastTask]);
end

% thread nummer labels
for j=1:1:lastTask+1
  for i=1:1:count
    b = num2str(n5(i,j));
    c = cellstr(b);
    dx = 0.01; dy = 0.1; % displacement so the text does not overlay the data points
    text(n3(i,j)+dx, n4(i,j)+dy, c);
  end
end
hold off;