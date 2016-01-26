% please run this script in Octave to get the graphs and statistics

function show_stats(filename, needHold, style, basename)

data = dlmread(filename);

numLines = size(data, 1);

linesArray = zeros(15*20, 5);
linesArrayIndex = 1;

cyclesArray = zeros(20, 8);
cyclesArrayIndex = 1;

for i = 1:numLines
	if(data(i,2) == 1)
		linesArray(linesArrayIndex,:) = data(i,3:7);
		linesArrayIndex++;
	else
		cyclesArray(cyclesArrayIndex,:) = data(i,3:10);
		cyclesArrayIndex++;
	endif
endfor

figure(1);
subplot(2,1,1);
if(needHold == 1)
	hold on;
else
	hold off;
endif
plot(linesArray(:,3), style);
title("Lines RAM, KiB");

subplot(2,1,2);
if(needHold == 1)
	hold on;
else
	hold off;
endif
plot(linesArray(:,2), style);
title("Line running time, ms");

print(strcat(basename, "_lines.pdf"), "-dpdf");

figure(2);
subplot(2,1,1);
if(needHold == 1)
	hold on;
else
	hold off;
endif
plot(cyclesArray(:,3), style);
title("Cycles RAM, KiB");

subplot(2,1,2);
if(needHold == 1)
	hold on;
else
	hold off;
endif
plot(cyclesArray(:,2), style);
title("Cycle running time, ms");

print(strcat(basename, "_cycles.pdf"), "-dpdf");

meanCycleTime = mean(cyclesArray(:,2));
stdCycleTime = std(cyclesArray(:,2));
meanLineTime = mean(linesArray(:,2));
stdLineTime = std(linesArray(:,2));

printf("Initial memory level: %f\n", linesArray(1,3));
printf("Cycle time: %f +- %f ms\n", meanCycleTime, stdCycleTime);
printf("Line time: %f +- %f ms\n", meanLineTime, stdLineTime);
printf("\n");
endfunction

show_stats("./log_0_3000_3000_0_0.txt", 0, '-k', 'report1');
show_stats("./log_1_3000_3000_0_0.txt", 1, '-r', 'report1');
show_stats("./log_1_3000_3000_50_0.txt", 1, '-b', 'report1');
