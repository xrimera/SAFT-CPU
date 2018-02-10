% Bildgröße x-Achse (Immer durch 4 teilbar. Vollständiges Bild kubisch)
voxelgenerator=2.^(2:0.1:7)
voxel=voxelgenerator.+mod(voxelgenerator,4);
% Anzahl der AScans
blocksize=1:1:22;
% Anzahl der Messungen pro Konfiguration
samples = 10;
% timetable
times= zeros(length(blocksize),length(voxel),1:1:samples,1:1:samples);

% Fixe Anzahl an threads
addsig2vol_3_mex(4);

for i=blocksize
    for j=1:length(voxel)
      
        % Rechne blocked-AScans
        count=i; senderPos = 0.01.*rand(3,count); receiverPos = 0.01.*rand(3,count); IMAGE_STARTPOINT = [0,0,0]; IMAGE_RESOLUTION= 0.001; TimeInterval=1e-7;
        DataLength=3000;
        Data=zeros(3000,count);
        Speed=1500+rand(1,count);
        Data(floor(DataLength.*rand(count-1,1))+1,1:count)=1;
        x=voxel(j); 
        imagesum=zeros([x,x,x]);

        for s=1:1:samples
          while 1
              tic;
              bild = addsig2vol_3_mex(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),imagesum);
              times(i,j,s,0)=toc;  if times(i,j,s,0)<10^8 break; end %%workaround for ugly times
          end
        end

        % Rechne unblocked-AScan
        count=1; senderPos = 0.01.*rand(3,count); receiverPos = 0.01.*rand(3,count); IMAGE_STARTPOINT = [0,0,0]; IMAGE_RESOLUTION= 0.001; TimeInterval=1e-7;
        DataLength=3000;
        Speed=1500+rand(1,count);
        Data=zeros(3000,count);
        Data(floor(DataLength.*rand(count,1)),1:count)=1;
        x=voxel(j); 
        imagesum=zeros([x,x,x]);

        for s=1:1:samples
            while 1
                tic;
                for i=1:i
                    imagesum = addsig2vol_3_mex(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),imagesum);
                end
                times(i,j,0,s)=toc;  if times(i,j,0,s)<10^8 break; end %%workaround for ugly times
            end
        end
        
    end
end

%plot(voxel',mean(squeeze(times(:,:,1)./times(:,:,2))))
%plot(voxel',voxel.^3./(mean(squeeze(times(:,:,1)./times(:,:,2)))))
%%%ascan blocked
%figure; imagesc(voxel,1:length(blocksize),(repmat((1:length(blocksize))',[1 length(voxel)]).*voxel.^3)./(times(1:length(blocksize),:,1))); colorbar;title('blocked')
%figure; plot(voxel,(repmat((1:length(blocksize))',[1 length(voxel)]).*voxel.^3)./(times(1:8,:,1)));title('blocked')


%%%SAFT von ascan unblocked, averaging time measurements
figure; imagesc(voxel,1:length(blocksize),(repmat((1:length(blocksize))',[1 length(voxel)]).*voxel.^3)./(times(1:length(blocksize),:,1))); colorbar;title('blocked')
figure; plot(voxel,(repmat((1:length(blocksize))',[1 length(voxel)]).*voxel.^3)./(times(1:8,:,1)));title('blocked')

%%%SAFT von ascan unblocked, averaging time measurements
figure; imagesc(voxel,1:length(blocksize),voxel.^3./mean((times(1:length(blocksize),:,2))./repmat((1:length(blocksize))',[1 length(voxel)]))  ); colorbar, title('unblocked')
figure; plot(voxel,repmat((1:length(blocksize))',[1 length(voxel)]).*voxel.^3./(times(1:length(blocksize),:,2)));title('unblocked')

