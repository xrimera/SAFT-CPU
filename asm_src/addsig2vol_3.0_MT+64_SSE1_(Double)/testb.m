
voxel=2.^(2:8);
times= zeros(64,length(voxel),2);

addsig2vol_3(2)

for i=1:128
    i
    for j=1:length(voxel)
        count=i; senderPos = 0.01.*rand(3,count); receiverPos = 0.01.*rand(3,count); IMAGE_STARTPOINT = [0,0,0]; IMAGE_RESOLUTION= 0.001; TimeInterval=1e-7;
        DataLength=3000;
        Data=zeros(3000,count);
        Speed=1500+rand(1,count);
        Data(floor(DataLength.*rand(count-1,1))+1,1:count)=1;
        x=voxel(j); imagesum=zeros([x,x,x]);

        while 1
            tic;
            bild = addsig2vol_3_mex(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),imagesum);
            times(i,j,1)=toc;
            if times(i,j,1)<10^8 break; end %%workaround for ugly times
        end

        count=1; senderPos = 0.01.*rand(3,count); receiverPos = 0.01.*rand(3,count); IMAGE_STARTPOINT = [0,0,0]; IMAGE_RESOLUTION= 0.001; TimeInterval=1e-7;
        DataLength=3000;
        Speed=1500+rand(1,count);
        Data=zeros(3000,count);
        Data(floor(DataLength.*rand(count,1)),1:count)=1;
        imagesum=zeros([x,x,x]);

        while 1
            tic;
            for i=1:i
                imagesum = addsig2vol_3_mex(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),imagesum);
            end
            times(i,j,2)=toc; if times(i,j,2)<10^8 break; end %%workaround for ugly times
        end

    end
end

plot(voxel',mean(squeeze(times(:,:,1)./times(:,:,2))))
