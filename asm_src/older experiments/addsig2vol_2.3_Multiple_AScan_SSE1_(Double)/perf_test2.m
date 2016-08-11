function empty=per_test()%
clear all
%dbstop if error

i=1; %block level

max_imageside= 2; %cubic %FIX NEEDED
amount = 1000000;
average_amount = 100000000; %minimum amount of memory to transfer
ascan_n = 3000;

empty=[];
unblocked=[];
steps = [1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8182];
average = 8;

blocked=zeros(length(steps),max_imageside);
unblocked = blocked;
image_size = max_imageside
image=zeros([image_size image_size image_size]);
image_n=uint32(size(image));
%image=zeros([2 2 2]);

rec_pos = single(ones(3,i));
vec_f = single(ones(3,1));
scalar_f=single(ones(1,1));
Ascan=rand(ascan_n,i);


tic;
for i=1:amount
end
empty(1)=toc/amount

tic;
for i=1:amount
    addsig2vol_2empty1([]);
end
empty(2)=toc/amount

tic;
for i=1:amount
    addsig2vol_2empty2(Ascan,vec_f,rec_pos,vec_f,scalar_f,scalar_f,vec_f,image_n,image );
end
empty(3)=toc/amount

tic;
for i=1:amount
    o=addsig2vol_2empty3(Ascan,vec_f,rec_pos,vec_f,scalar_f,scalar_f,vec_f,image_n,image );end
empty(4)=toc/amount

amount =amount /10;

tic;
for i=1:amount
    o=addsig2vol_2(Ascan,vec_f,rec_pos,vec_f,scalar_f,scalar_f,vec_f,image_n,image );end
empty(5)=toc/amount

% amount =amount /10;
% image_size = max_imageside
% image=zeros([32 32 1]);
% image_n=uint32([size(image) 1]);
% 
% tic;
% for i=1:amount
%     o=addsig2vol_2(Ascan,vec_f,rec_pos,vec_f,scalar_f,scalar_f,vec_f,image_n,image );end
% empty(6)=toc/amount
% 
% 
% amount =amount /10;
% image_size = max_imageside
% image=zeros([64 64 64]);
% image_n=uint32([size(image)]);
% 
% tic;
% for i=1:amount
%     o=addsig2vol_2(Ascan,vec_f,rec_pos,vec_f,scalar_f,scalar_f,vec_f,image_n,image );end
% empty(7)=toc/amount


bar(empty);

disp('');




% max_imageside= 64; %cubic
% blocked=[];
% unblocked=[];
% steps = [1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8182];
% average = 8;
% average_amount = 100000000; %minimum amount of memory to transfer
% 
% blocked=zeros(length(steps),max_imageside);
% unblocked = blocked;
% 
% 
% 
% for image_size = 2:max_imageside
%     image=zeros([image_size image_size image_size]);
%     image_size
%     
%     for i= steps
%         Ascan=rand(3000,i);
%         rec_pos = single(ones(3,i));
%         
%         average=ceil(average_amount/(3000*8*i+image_size^3*8))
%         
%         
%         %blocked
%         time=0;
%         tic;
%         for j=1:average
%             image_out=addsig2vol_2(Ascan,vec_f,rec_pos,vec_f,scalar_f,scalar_f,vec_f,image_n,image );
%         end
%         blocked(i,image_size)=toc/average;
%         
%         %unblocked
%         time=0;
%         tic;
%         for j=1:average
%             for k=1:i %emulate number of blocks by single calls
%                 image_out=addsig2vol_2(Ascan(:,k),vec_f,rec_pos(:,k),vec_f,scalar_f,scalar_f,vec_f,image_n,image );
%             end
%         end
%         unblocked(i,image_size)=toc/average;
%         
%     end
%  
% end
% 
% q=(unblocked(steps,:)./blocked(steps,:));
% q(isnan(q))=0;
% q(isinf(q))=0;
% figure; imagesc(mean(q,1));
% figure; imagesc(mean(q,2));
% 
% figure; imagesc(blocked(steps,:));
% figure; imagesc(unblocked(steps,:));