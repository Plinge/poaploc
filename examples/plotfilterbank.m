freq =  [0.3000  0.3678  0.4443 0.5306 0.6279 0.7378 0.8617 1.0015 ...
         1.1592 1.3371 1.5378 1.7643 2.0198 2.3080 2.6332 3.0000] * 1.0e+03;
     
bws = [1,2,4];
colors = 'rgb';


for bw = 1:3
    figure();
    title(['bandwidth ' num2str(bws(bw))]);
    hold on;
    f = linspace(0,3000*1.5,5000);
    for i=1:16
        g = abs(gammatone(f,freqs(i),bws(bw)));
        plot(f,g);
    end
    xlim([0 max(f)]);
    ylim([0 1]);
    hold off;     
end