hdr;

k=0;
for N=8:32:2000; k=k+1;

   A1=rand(N,N); B1=rand(N,N); 
   A2=rand(N,N); B2=rand(N,N);
   A3=rand(N,N); B3=rand(N,N);
   A4=rand(N,N); B4=rand(N,N);
   C1=A1*B1; C2=A2*B2; C3=A3*B3; C4=A4*B4; % Warm start

   t0=tic;                                 %  8X loop
     C1=A1*B1; C2=A2*B2; C3=A3*B3; C4=A4*B4;
     A1=C1*B1; A2=C2*B2; A3=C3*B3; A4=C4*B4;
   etime = toc(t0);

   N3 = N^3;
   gflops = (8*2*N3/etime)/1e9;

   ncore = 8;
   gcore = gflops / ncore;                % GFLOPS / core

   disp([ N etime gcore ]);

   NN(k)=N;
   gc(k)=gcore;

end;

plot(NN,gc,'ro-',lw,2);
xlabel('Matrix Size, N',fs,20);
ylabel('GFLOPS',fs,20);
title ('Matlab C=AB GFLOPS per Core',fs,20);

