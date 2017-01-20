
function y = gammatone(f,fb,wb)
%% [2] R. Patterson, I. Nimmo-Smith, J. Holdsworth und P. Rice
%   An ef?cient auditory ?lterbank based on the gammatone functions. 
%   Tech.Rep. APU Report 2341, MRC, Applied Psychology Unit, Cambridge U.K, 1988.
%  [3] M. Slaney: An ef?cient implementation of the Patterson-Holdsworth auditory ?lter bank.
%   Tech.Rep. 35, Apple Computer, Inc., 1993. 
    C1 = 24.673; C2 = 4.368;    
%% [1] B. Glasberg und B. Moore
%   Derivation of auditory ?lter shapes from notched-noise data.
%   Hearing Research, 47(1–2):103–138, August 1990. 
    thebw =  C1 * ((C2 * 1e-3* fb)+1.0);
%% [4] M. Unoki and M. Akagi
%   A method of signal extraction from noisy signal based on auditory scene analysis
%   Speech Commun., vol. 27, no. 3, pp. 261–279, 1999.    
    y = (1+j*(f-fb)/(thebw*wb)).^(-4);
    % compute absolute directly
    %b = thebw*wb; d = (fb - f);	n1 = b.^4;
	%d1 = b.^4 - 6* b.^2* d.^2 + d.^4;
	%d2 = 4*(b * d.^3 - b.^3 * d);
	%y = n1 * ( d1.^2 + d2.^2 ).^(-0.5);	
end