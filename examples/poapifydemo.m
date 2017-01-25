% demo of command line calling poapify
clear all; close all;
%% global parameters
BINPATH = '/Users/pli/bin/'; % c:\local_bin\
INPUT_FILE = 'sentence.wav';
OUTPUT_FILE = 'spiked.wav';
BANDWIDTH = 4.0;
FRANGE = [300 3000];
BANDS = 16;
%% read and plot input
figure();
[input, fsin] = audioread(INPUT_FILE);
if size(input,1)/fsin > 10
    input = input(1:fsin*10,:);
end
plot(linspace(0, size(input,1)/fsin, size(input,1)),  input);
title('input');
inplay= audioplayer(input,fsin);
playblocking(inplay);
%% run command
command = [BINPATH 'poapify --spike-mode 1' ...
    ' --bandwidth ' num2str(BANDWIDTH) ...
    ' --bands ' num2str(BANDS)...
    ' --fmin ' num2str(FRANGE(1)) ' --fmax ' num2str(FRANGE(2)) ...
    ' ' INPUT_FILE ' ' OUTPUT_FILE ];
[status, cmdout] = system(command);
%% check output
if status ~= 0    
    disp(cmdout);
    error('%s\ngot code %d!', command, status);
end
%% parse output
lines = strsplit(cmdout,'\n');
for i=1:size(lines,2)
    if strfind(lines{1,i}, 'Center frequencies')
        words = strsplit(lines{1,i});
        words = words(3:size(words,2));
        freqs = zeros(size(words,2),1);
        for j=1:size(words,2)
            freqs(j) = str2double(words{1,j});
        end
    end
end

%% read and plot output
figure();
[output, fsout] = audioread(OUTPUT_FILE);
bands = size(freqs,1);
for i=1:bands
    subplot(bands/2,2,i);
    plot(linspace(0, size(output,1)/fsout, size(output,1)), output(:,i));
    title(['spikes ' num2str(freqs(i)) '  Hz']);
end
downmix = mean(output,2);
outplay = audioplayer(downmix,fsout);
playblocking(outplay);