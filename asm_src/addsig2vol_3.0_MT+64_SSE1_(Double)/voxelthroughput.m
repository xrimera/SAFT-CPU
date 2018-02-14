clear all()
close all
% Bildgröße x-Achse (Immer durch 4 teilbar. Vollständiges Bild kubisch)
voxelgenerator=unique(round(2.^(2:0.1:8)))
voxel=voxelgenerator.+(4.-mod(voxelgenerator,4));
voxel=unique(voxel)
% Anzahl der AScans
blocksize=1:5:31;
% Anzahl der Messungen pro Konfiguration
samples = 20;
AscanLength = 3000;
% timetable
timesBlocked = zeros(length(blocksize),length(voxel),1:1:samples);
timesUnblocked = zeros(length(voxel),1:1:samples);

% Fixe Anzahl an threads
addsig2vol_3_mex(4);

% Rechne blocked-AScans
for i=1:length(blocksize)
    for j=1:length(voxel)
        count=blocksize(i); senderPos = 0.01.*rand(3,count); receiverPos = 0.01.*rand(3,count); IMAGE_STARTPOINT = [0,0,0]; IMAGE_RESOLUTION= 0.001; TimeInterval=1e-7;
        DataLength=AscanLength;
        Data=zeros(AscanLength,count);
        Speed=1500+rand(1,count);
        %Data(floor(DataLength.*rand(count-1,1))+1,1:count)=1;
        x=voxel(j); 
        Data = rand(AscanLength,count);
        imagesum=zeros([x,x,x]);

        for s=1:1:samples
            while 1
                tic;
                [bild, buffer]= addsig2vol_3_mex(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),imagesum);
                timesBlocked(i,j,s)=toc,  if timesBlocked(i,j,s)<10^8 break; end %%workaround for ugly times
            end
        end
    end
end

 % Rechne unblocked-AScan
for j=1:length(voxel)
      count=1; senderPos = 0.01.*rand(3,count); receiverPos = 0.01.*rand(3,count); IMAGE_STARTPOINT = [0,0,0]; IMAGE_RESOLUTION= 0.001; TimeInterval=1e-7;
      DataLength=AscanLength;
      Speed=1500+rand(1,count);
      Data=zeros(AscanLength,count);
      x=voxel(j); 
      Data = rand(AscanLength,count);
      imagesum=zeros([x,x,x]);

      for s=1:1:samples
          while 1
              tic;
              [bild, buffer] = addsig2vol_3_mex(Data,single(IMAGE_STARTPOINT),single(receiverPos),single(senderPos),single(Speed),single(IMAGE_RESOLUTION),single(TimeInterval),uint32([x,x,x]),imagesum);
              timesUnblocked(j,s)=toc;  if timesUnblocked(j,s)<10^8 break; end %%workaround for ugly times
          end
      end
end

%%%SAFT von ascan unblocked, averaging time measurements
figure; imagesc(voxel,blocksize,(repmat(blocksize',[1 length(voxel)]).*voxel.^3)./mean(timesBlocked,3)); colorbar;title('blocked')
figure; plot(voxel,(repmat(blocksize',[1 length(voxel)]).*voxel.^3)./mean(timesBlocked,3));title('blocked')

B = mean(timesBlocked,3);
figure; plot(voxel,B');title('blocked')

%%%SAFT von ascan unblocked, averaging time measurements
%figure; imagesc(voxel,1:length(blocksize),voxel.^3./mean((times(1:length(blocksize),:,2))./repmat((1:length(blocksize))',[1 length(voxel)]))  ); colorbar, title('unblocked')
figure; plot(voxel,(voxel.^3)./mean(timesUnblocked,2)');title('unblocked')
