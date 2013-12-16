%
	theta = 0;
	r = 0;

	theta_step = 5.8;
	r_step = 0.025;

	num_pts = 50;

	xs = zeros(num_pts,1);
	ys = zeros(num_pts,1);

	for ind = 1 : num_pts
		% increase angle
		theta = theta + theta_step;

		% increase distance
		r = r + r_step;	

		xs(ind) = r*sin(theta);
		ys(ind) = r*cos(theta);

	end

	figure
	plot(xs,ys,'.');
	axis equal
	title(sprintf('theta step: %f     r step: %f', theta_step, r_step));


	disp(sprintf('static const unsigned int num_pts = %d;',num_pts));

	disp(sprintf('\n'));
	disp(sprintf('double spiral_xs[num_pts] = {'));
	for ind = 1 : num_pts
		if (ind == num_pts)
			disp(sprintf('%3.58f',xs(ind)));
		else
			disp(sprintf('%3.58f,',xs(ind)));
		end
	end
	disp(sprintf('};'));

	disp(sprintf('\n'));

	disp(sprintf('double spiral_ys[num_pts] = {'));
	for ind = 1 : num_pts
		if (ind == num_pts)
			disp(sprintf('%3.58f',ys(ind)));
		else
			disp(sprintf('%3.58f,',ys(ind)));
		end
	end
	disp(sprintf('};'));
