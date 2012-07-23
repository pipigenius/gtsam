%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% GTSAM Copyright 2010, Georgia Tech Research Corporation,
% Atlanta, Georgia 30332-0415
% All Rights Reserved
% Authors: Frank Dellaert, et al. (see THANKS for the full author list)
%
% See LICENSE for the license information
%
% @brief Read graph from file and perform GraphSLAM
% @author Frank Dellaert
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Find data file
N = 2500;
% dataset = 'sphere_smallnoise.graph';
% dataset = 'sphere2500_groundtruth.txt';
dataset = 'sphere2500.txt';

datafile = findExampleDataFile(dataset);

%% Initialize graph, initial estimate, and odometry noise
import gtsam.*
model = noiseModel.Diagonal.Sigmas([0.05; 0.05; 0.05; 5*pi/180; 5*pi/180; 5*pi/180]);
[graph,initial]=load3D(datafile,model,true,N);

%% Plot Initial Estimate
cla
first = initial.at(0);
plot3(first.x(),first.y(),first.z(),'r*'); hold on
plot3DTrajectory(initial,'g-',false);

%% Read again, now with all constraints, and optimize
import gtsam.*
graph = load3D(datafile, model, false, N);
graph.add(NonlinearEqualityPose3(0, first));
optimizer = DoglegOptimizer(graph, initial);
result = optimizer.optimizeSafely();
plot3DTrajectory(result, 'r-', false); axis equal;

view(0); axis equal;