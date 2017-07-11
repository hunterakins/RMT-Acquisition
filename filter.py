

def butter_bandpass(lowcut, highcut, samplingrate, order=4):
    nyq = 0.5 * samplingrate
    low = lowcut / nyq
    high = highcut / nyq
    if high >=1. and low == 0.:
        b = np.array([1.])
        a = np.array([1.])

    elif high < 0.95 and low > 0. :
        wp = [1.05*low,high-0.05]
        ws = [0.95*low,high+0.05]
        order,wn = buttord(wp,ws,0., 30.)
        b, a = butter(order, wn, btype='band')
    
    elif high>= 0.95:
        order,wn = buttord( 15*low,0.05*low,gpass=0.0, gstop=10.0)
        b, a = butter(order, wn, btype='high')
    elif low <= 0.05:
        order,wn = buttord( high-0.05,high+0.05,gpass=0.0, gstop=10.0)
        b, a = butter(order, wn, btype='low')

    return b, a



def butter_bandpass_filter(data, lowcut, highcut, samplingrate, order=4):
    b, a = butter_bandpass(lowcut, highcut, samplingrate, order=order)
    y = lfilter(b, a, data)
    return y

