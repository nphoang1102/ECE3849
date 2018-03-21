function [R,U] = response_time(T,E)
% [R,U] = response_time(T,E)
% Calculate the response time of periodic real-time tasks.
% Arguments:
%   T - vector of task periods
%   E - vector of task execution times (same dimensions as T)
% Returns:
%   R - vector of response times
%   U - overall CPU utilization
R = zeros(size(T));
for i = 1:length(R)
	R(i) = sum(E(1:i));
	R_old = 0;
	j = 1:i-1;
	while abs(R(i) - R_old) > 10*eps(R(i))
		R_old = R(i);
		R(i) = E(i) + sum(ceil(R(i)./T(j)) .* E(j));
	end
end
U = sum(E./T); % CPU utilization

