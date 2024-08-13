//    Copyright 2022, farm-ng inc.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#pragma once

#include "farm_ng/core/proto_conv/traits.h"
#include "farm_ng/core/sensor.pb.h"
#include "sophus2/sensor/camera_model.h"
#include "sophus2/sensor/camera_rig.h"
#include "sophus2/sensor/clipping_planes.h"

namespace farm_ng {

FARM_PROTO_CONV_TRAIT(sophus2::CameraModel, core::proto::CameraModel);
FARM_PROTO_CONV_TRAIT(
    std::vector<sophus2::CameraModel>, core::proto::CameraModels);
FARM_PROTO_CONV_TRAIT(sophus2::RigidCamera, core::proto::RigidCamera);
FARM_PROTO_CONV_TRAIT(sophus2::MultiCameraRig, core::proto::MultiCameraRig);
FARM_PROTO_CONV_TRAIT(sophus2::ClippingPlanes, core::proto::ClippingPlanes);

}  // namespace farm_ng
