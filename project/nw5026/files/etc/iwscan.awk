#iw wlan0 scan|sed -e 's#(on wlan0# (on wlan0#g' |awk -f /etc/iwscan.awk >/tmp/aplist.txt
BEGIN {
	system("rm /tmp/scan.done");
	("uci get wifilist.number2g.num")|getline l; #print l;
	#printf "l====%s",l;
	i=0;
	for(j=0;j<l;j++)
	{	
		
		cmd=("uci get wifilist.@wifi2g["j"].ssid");
		#printf "cmd=%s\n",cmd;
		cmd|getline y;
		#print y
		save_ssid[j]=y;
		#printf "save_ssid=%s\n",save_ssid[i];
	}
}
$1 == "BSS" {
if($2 != "Load:"){
	i++;
	mac[i]=$2;
	split(mac[i],mm,":")
	mac[i]=mm[1]""mm[2]""mm[3]""mm[4]""mm[5]""mm[6]
	#print mac[i];
	enc[i]="NONE"
}
}
$1 == "SSID:" {
	ssid=$0;

	#system("uci get wifilist.@wifi2g["j"].ssid")|getline x;
	#printf "x===%s",x;
	#printf ${#ssid:0:6} 
	#split(ssid,ss,":");
	record[i]=0;		
	ssid[i]=substr(ssid,8,32);
	
	for(j=0;j<l;j++){
		#print ssid[i];
		#print save_ssid[j];
	
		if(ssid[i]==save_ssid[j])
			record[i]=1;
	}
	#print record[i];
	#echo ${ssid:6:32};
	#ssid[i]=$2;
}
$1 == "freq:" {
	freq[i]=$NF;
#if(freq[i]<5000){
	if(freq[i]==2412)
		chan[i]=1;
	else if(freq[i]==2417)
		chan[i]=2;
	else if(freq[i]==2422)
		chan[i]=3;
	else if(freq[i]==2427)
		chan[i]=4;
	else if(freq[i]==2432)
		chan[i]=5;
	else if(freq[i]==2437)
		chan[i]=6;
	else if(freq[i]==2442)
		chan[i]=7;
	else if(freq[i]==2447)
		chan[i]=8;
	else if(freq[i]==2452)
		chan[i]=9;
	else if(freq[i]==2457)
		chan[i]=10;
	else if(freq[i]==2462)
		chan[i]=11;
	else if(freq[i]==2467)
		chan[i]=12;
	else if(freq[i]==2472)
		chan[i]=13;
	else if(freq[i]==2484)
		chan[i]=14;
#else {
        if(freq[i]==5745)         
                chan[i]=149;        
        else if(freq[i]==5765)    
                chan[i]=153;        
        else if(freq[i]==5785)    
                chan[i]=157;        
        else if(freq[i]==5805)    
                chan[i]=161;        
        else if(freq[i]==5825)
                chan[i]=165;    
        else if(freq[i]==5180)
                chan[i]=36;    
        else if(freq[i]==5190)
                chan[i]=38;    
        else if(freq[i]==5200)
                chan[i]=40;    
        else if(freq[i]==5210)
                chan[i]=42;    
        else if(freq[i]==5220)
                chan[i]=44;   
        else if(freq[i]==5230)
                chan[i]=46;    
        else if(freq[i]==5240) 
                chan[i]=48;    
        else if(freq[i]==5260) 
                chan[i]=52;    
        else if(freq[i]==5280) 
                chan[i]=56;    
        else if(freq[i]==5300) 
                chan[i]=60;    
        else if(freq[i]==5320) 
                chan[i]=64;    
#}
#	printf "%s\n",$NF;
}
$1 == "signal:" {
	sig[i]=$2
	split(sig[i],sss,".");
	sig[i]=sss[1];
}
$1 == "WPA:" { 
if(enc[i]=="WPA2-PSK")
	enc[i]="WPA/WPA2-PSK"
else enc[i]="WPA-PSK"
}
$1 == "RSN:" { 
if(enc[i]=="WPA-PSK")
	enc[i]="WPA/WPA2-PSK"
else enc[i]="WPA2-PSK"
}

$5 == "802.1X" { 
	#if(enc[i] == "NONE" )
		enc[i]="WPA2-1X"
}
$1 == "WEP:" {
	enc[i]="WEP"
}
$2 == "Pairwise" {
	if($4 == "TKIP") ta1="tkip";
	if($4 == "CCMP") ta1="aes";
	
	if($5 != "")
	{
		if($5 == "TKIP") ta2="tkip";
		if($5 == "CCMP") ta2="aes";

		cipher[i]=ta1"/"ta2;
	}
		else cipher[i]=ta1;
	}
#$4 == "channel" {
#	chan[i]=$5;
#}
#i++;
#echo '}'>>ap.txt
END {
    #printf "%s\t\t\t%s\t\t\t%s\t%s\t%s\t%s\t%s\n","SSID","MAC","channel","Frequency","Signal","Encryption","Cipher"
    len=i;
    printf "%s","<APList>"
    for(i=1;i<len+1;i++){
	#print i,mac[i],ssid[i],freq[i],sig[i],enc[i],cipher[i];
        #printf "%s\t\t\t%s\t\t\t%s\t%s\t%s\t%s\t%s\n",ssid[i],mac[i],chan[i],freq[i],sig[i],enc[i],cipher[i];
	printf "<AP name=\"%s\" mac=\"%s\" channel=\"%s\" encrypt=\"%s\" rssi=\"%s\" tkip_aes=\"%s\" record=\"%s\" />",ssid[i],mac[i],chan[i],enc[i],sig[i],cipher[i],record[i];
	#printf "{"ssid":"%s","mac":"%s",}"
}
    print "</APList>";#<Return status=\"true\" ></Return></getSysInfo>"
	system("touch /tmp/scan.done");
    #for (w in wifi) {
#	print w,wifi[w];
#	e=wifi[w];
#	printf "w=%s,ssid=%s",w,w["SSID"];
        #printf "%s\t\t%s\t\t%s\t%s\n",e["SSID"],e["freq"],e["sig"],e["enc"]
#    }
}
