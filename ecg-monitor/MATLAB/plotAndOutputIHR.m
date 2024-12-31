function plotAndOutputIHR(portIn, Fs, windowWidth)
%plotAndOutputIHR Plot Incoming ECG Waveform and Output Instantaneous Heart
% Rate. Fs in Hz (set to 1/Ticker period) and windowWidth in seconds.

    port = serialport(portIn,9600); % initialize serial port

    datapoints = []; % array of data
    time = []; % array of times
    numPoints = 0; % counting number of points

    t1 = -1;
    onPeak = false;
    maxIndex = 1;
    maxValue = 0;
    calculated = true;
    threshold = 60000;

    % peak arrays and times
    peaks = [];
    peakTimes = [];

    xWidth = Fs*windowWidth; % calculate windowWidth in indices using Fs

    figure % new figure each time, can comment out

    % loop forever, hit ctrl+c in MATLAB window to cancel

    while(true)
        datapoint = str2double(port.readline); % get datapoint, one per line
        
        datapoints = [datapoints,datapoint]; % save datapoint
        time = [time,numPoints*1/(Fs)]; % update time array
        numPoints = numPoints + 1; % increment number of points

        % plot
        plot(time,datapoints);

        % adjust to window width
        if (xWidth < numPoints)
            xlim([time(numPoints - xWidth) time(numPoints)])
        else
            xlim([0 windowWidth]);
        end
        
        % maintain full y axis range, feel free to adjust
        ylim([0 70000]);

        % Peak detector example: output when value is over 40k. 
        % YOUR JOB: update this so that peaks and peakTimes only get added
        % to for an actual R peak.
       
        if datapoint > threshold
            peaks = [peaks,datapoint];
            peakTimes = [peakTimes,time(end)];
            if onPeak == false %initializes first point of any peak
                maxIndex = numPoints;
                maxValue = datapoint;
            end
            onPeak = true;
        else
            if onPeak == true
                calculated = false;
            end
            onPeak = false;
        end
        
        if onPeak == true
            if datapoint > maxValue
                maxValue = datapoint;
                maxIndex = numPoints;
            end
        end  
        if calculated == false
            if (t1 >= 0)
                t2 = time(maxIndex);
                IHR = 1/(t2 - t1)*60;
                fprintf("IHR = %.1f\n",IHR);
                fprintf("t1 = %.1f\n",t1);
                fprintf("t2 = %.1f\n\n",t2);
                t1 = t2;
            else
                t1 = time(maxIndex);
            end
            calculated = true;
        end
        
        % plot peaks
        hold on
        plot(peakTimes,peaks,'Color','red','Marker','o','LineStyle','none');
        hold off
        
        

        % Code seems to slow down a lot with these on...feel free to try it
        % xlabel('Time (seconds');
        % ylabel('ADC count');
    end

end