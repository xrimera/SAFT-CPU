figure; 
hold on; 

a = 1;
blocksize(a)
plot(voxel,(repmat(blocksize(a)',[1 length(voxel)]).*voxel.^3)./mean(stickyTimes,3)(a,:), '-'); title('blocked'); 
plot(voxel,(repmat(blocksize(a)',[1 length(voxel)]).*voxel.^3)./mean(origTimes,3)(a,:), '.-.'); title('blocked'); 
a = 2;
blocksize(a)
plot(voxel,(repmat(blocksize(a)',[1 length(voxel)]).*voxel.^3)./mean(stickyTimes,3)(a,:), '-'); title('blocked'); 
plot(voxel,(repmat(blocksize(a)',[1 length(voxel)]).*voxel.^3)./mean(origTimes,3)(a,:), '.-.'); title('blocked'); 

a = 7;
blocksize(a)
plot(voxel,(repmat(blocksize(a)',[1 length(voxel)]).*voxel.^3)./mean(stickyTimes,3)(a,:), '-'); title('blocked'); 
plot(voxel,(repmat(blocksize(a)',[1 length(voxel)]).*voxel.^3)./mean(origTimes,3)(a,:), '.-.'); title('blocked'); 


legend("threads gestaffelt, 1 AScan", "original, 1 AScan", "threads gestaffelt, 6 AScan", "original, 6 AScan", "threads gestaffelt, 31 AScan", "original, 31 AScan");
