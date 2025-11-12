/*
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RayArApplication.h"
#include "Asset/AssetManager.h"
#include <android/asset_manager.h>

#include <array>

#include "arcore_c_api.h"
#include "PlaneRenderer.h"
#include "Util.h"

namespace Ray
{
    namespace
    {
        constexpr size_t k_MaxNumberOfObjectsToRender = 20;
        const glm::vec3 k_White = {255, 255, 255};

        // Assumed distance from the device camera to the surface on which user will
        // try to place objects. This value affects the apparent scale of objects
        // while the tracking method of the Instant Placement point is
        // SCREENSPACE_WITH_APPROXIMATE_DISTANCE. Values in the [0.2, 2.0] meter
        // range are a good choice for most AR experiences. Use lower values for AR
        // experiences where users are expected to place objects on surfaces close
        // to the camera. Use larger values for experiences where the user will
        // likely be standing and trying to place an object on the ground or floor
        // in front of them.
        constexpr float k_ApproximateDistanceMeters = 1.0f;

        void SetColor(float r, float g, float b, float a, float* color4f)
        {
            color4f[0] = r;
            color4f[1] = g;
            color4f[2] = b;
            color4f[3] = a;
        }
    }  // namespace

    RayArApplication::RayArApplication(AAssetManager* assetManager)
        : m_assetManager(assetManager)
    {
        AssetManager::Init(assetManager);
    }

    RayArApplication::~RayArApplication()
    {
        if (m_arSession != nullptr)
        {
            ArSession_destroy(m_arSession);
            ArFrame_destroy(m_arFrame);
        }
    }

    void RayArApplication::OnPause()
    {
        LOGI("OnPause()");
        if (m_arSession != nullptr)
            ArSession_pause(m_arSession);
    }

    void RayArApplication::OnResume(JNIEnv* env, void* context, void* activity)
    {
        LOGI("OnResume()");

        if (m_arSession == nullptr)
        {
            ArInstallStatus installStatus;
            // If install was not yet requested, that means that we are resuming the
            // activity first time because of explicit user interaction (such as
            // launching the application)
            bool userRequestedInstall = !m_installRequested;

            // === ATTENTION!  ATTENTION!  ATTENTION! ===
            // This method can and will fail in user-facing situations.  Your
            // application must handle these cases at least somewhat gracefully.
            CHECK_AND_THROW(ArCoreApk_requestInstall(env,activity,userRequestedInstall,&installStatus) == AR_SUCCESS, env,
                            "Please install Google Play Services for AR (ARCore).");

            switch (installStatus)
            {
                case AR_INSTALL_STATUS_INSTALLED: break;
                case AR_INSTALL_STATUS_INSTALL_REQUESTED:
                    m_installRequested = true;
                    return;
            }

            // === ATTENTION!  ATTENTION!  ATTENTION! ===
            // This method can and will fail in user-facing situations.  Your
            // application must handle these cases at least somewhat gracefully.  See
            // HelloAR Java sample code for reasonable behavior.
            CHECK_AND_THROW(ArSession_create(env, context, &m_arSession) == AR_SUCCESS, env,
                            "Failed to create AR session.");

            ConfigureSession();
            ArFrame_create(m_arSession, &m_arFrame);

            ArSession_setDisplayGeometry(m_arSession, m_displayRotation, m_width, m_height);
        }

        const ArStatus status = ArSession_resume(m_arSession);
        CHECK_AND_THROW(status == AR_SUCCESS, env, "Failed to resume AR session.");
    }

    void RayArApplication::OnSurfaceCreated()
    {
        LOGI("OnSurfaceCreated()");

        m_depthTexture.CreateOnGlThread();
        m_backgroundRenderer.InitializeGlContent(m_assetManager,
                                                 m_depthTexture.GetTextureId());
        m_pointCloudRenderer.InitializeGlContent(m_assetManager);
        m_objRenderer.InitializeGlContent(m_assetManager,
                                          "models/andy.obj",
                                          "models/andy.png");
        m_objRenderer.SetDepthTexture(m_depthTexture.GetTextureId(),
                                      m_depthTexture.GetWidth(),
                                      m_depthTexture.GetHeight());
        m_planeRenderer.InitializeGlContent(m_assetManager);
    }

    void RayArApplication::OnDisplayGeometryChanged(int displayRotation, int width, int height)
    {
        LOGI("OnSurfaceChanged(%d, %d)", width, height);
        glViewport(0, 0, width, height);
        m_displayRotation = displayRotation;
        m_width = width;
        m_height = height;
        if (m_arSession != nullptr)
            ArSession_setDisplayGeometry(m_arSession, displayRotation, width, height);
    }

    void RayArApplication::OnDrawFrame(bool depthColorVisualizationEnabled, bool useDepthForOcclusion)
    {
        // Render the scene.
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        if (m_arSession == nullptr) return;

        ArSession_setCameraTextureName(m_arSession,m_backgroundRenderer.GetTextureId());

        // Update session to get current frame and render camera background.
        if (ArSession_update(m_arSession, m_arFrame) != AR_SUCCESS)
            LOGE("RayArApplication::OnDrawFrame ArSession_update error");

        m_objRenderer.SetDepthTexture(m_depthTexture.GetTextureId(),
                                      m_depthTexture.GetWidth(),
                                      m_depthTexture.GetHeight());

        ArCamera* arCamera = nullptr;
        ArFrame_acquireCamera(m_arSession, m_arFrame, &arCamera);

        int32_t displayGeometryChanged = 0;
        ArFrame_getDisplayGeometryChanged(m_arSession, m_arFrame,&displayGeometryChanged);
        if (displayGeometryChanged != 0 || !m_calculateUvTransform)
        {
            // The UV Transform represents the transformation between screenspace in
            // normalized units and screenspace in units of pixels.  Having the size of
            // each pixel is necessary in the virtual object shader, to perform
            // kernel-based blur effects.
            m_calculateUvTransform = false;
            glm::mat3 transform = GetTextureTransformMatrix(m_arSession, m_arFrame);
            m_objRenderer.SetUvTransformMatrix(transform);
        }

        glm::mat4 viewMat;
        glm::mat4 projectionMat;
        ArCamera_getViewMatrix(m_arSession, arCamera, glm::value_ptr(viewMat));
        ArCamera_getProjectionMatrix(m_arSession, arCamera,0.1f, 100.f,
                                     glm::value_ptr(projectionMat));

        m_backgroundRenderer.Draw(m_arSession, m_arFrame,
                                  depthColorVisualizationEnabled);

        ArTrackingState arTrackingState;
        ArCamera_getTrackingState(m_arSession, arCamera, &arTrackingState);
        ArCamera_release(arCamera);

        // If the camera isn't tracking don't bother rendering other objects.
        if (arTrackingState != AR_TRACKING_STATE_TRACKING) return;

        int32_t is_depth_supported = 0;
        ArSession_isDepthModeSupported(m_arSession, AR_DEPTH_MODE_AUTOMATIC,
                                       &is_depth_supported);
        if (is_depth_supported)
            m_depthTexture.UpdateWithDepthImageOnGlThread(*m_arSession, *m_arFrame);

        // Get light estimation value.
        ArLightEstimate* arLightEstimate;
        ArLightEstimateState lightEstimateState;
        ArLightEstimate_create(m_arSession, &arLightEstimate);

        ArFrame_getLightEstimate(m_arSession, m_arFrame, arLightEstimate);
        ArLightEstimate_getState(m_arSession, arLightEstimate,
                                 &lightEstimateState);

        // Set light intensity to default. Intensity value ranges from 0.0f to 1.0f.
        // The first three components are Color scaling factors.
        // The last one is the average pixel intensity in gamma space.
        float colorCorrection[4] = {1.f, 1.f, 1.f, 1.f};
        if (lightEstimateState == AR_LIGHT_ESTIMATE_STATE_VALID)
        {
            ArLightEstimate_getColorCorrection(m_arSession, arLightEstimate,
                                               colorCorrection);
        }

        ArLightEstimate_destroy(arLightEstimate);
        arLightEstimate = nullptr;

        // Update and render planes.
        ArTrackableList* planeList = nullptr;
        ArTrackableList_create(m_arSession, &planeList);
        CHECK(planeList != nullptr);

        ArTrackableType planeTrackedType = AR_TRACKABLE_PLANE;
        ArSession_getAllTrackables(m_arSession, planeTrackedType, planeList);

        int32_t planeListSize = 0;
        ArTrackableList_getSize(m_arSession, planeList, &planeListSize);
        m_detectedPlanesCount = planeListSize;

        for (int i = 0; i < planeListSize; ++i)
        {
            ArTrackable* arTrackable = nullptr;
            ArTrackableList_acquireItem(m_arSession, planeList, i, &arTrackable);
            ArPlane* arPlane = ArAsPlane(arTrackable);
            ArTrackingState outTrackingState;
            ArTrackable_getTrackingState(m_arSession, arTrackable,&outTrackingState);

            ArPlane* subsumePlane;
            ArPlane_acquireSubsumedBy(m_arSession, arPlane, &subsumePlane);
            if (subsumePlane != nullptr)
            {
                ArTrackable_release(ArAsTrackable(subsumePlane));
                ArTrackable_release(arTrackable);
                continue;
            }

            if (ArTrackingState::AR_TRACKING_STATE_TRACKING != outTrackingState)
            {
                ArTrackable_release(arTrackable);
                continue;
            }

            m_planeRenderer.Draw(projectionMat, viewMat, *m_arSession, *arPlane);
            ArTrackable_release(arTrackable);
        }

        ArTrackableList_destroy(planeList);
        planeList = nullptr;

        m_objRenderer.SetUseDepthForOcclusion(m_assetManager, useDepthForOcclusion);

        // Render Andy objects.
        glm::mat4 modelMat(1.0f);
        for (auto& coloredAnchor : m_anchors)
        {
            ArTrackingState trackingState = AR_TRACKING_STATE_STOPPED;
            ArAnchor_getTrackingState(m_arSession, coloredAnchor.Anchor,
                                      &trackingState);
            if (trackingState == AR_TRACKING_STATE_TRACKING)
            {
                UpdateAnchorColor(&coloredAnchor);
                // Render object only if the tracking state is AR_TRACKING_STATE_TRACKING.
                Util::GetTransformMatrixFromAnchor(*coloredAnchor.Anchor, m_arSession,
                                                   &modelMat);
                m_objRenderer.Draw(projectionMat, viewMat, modelMat, colorCorrection,
                                   coloredAnchor.Color);
            }
        }

        // Update and render point cloud.
        ArPointCloud* arPointCloud = nullptr;
        ArStatus pointCloudStatus = ArFrame_acquirePointCloud(m_arSession, m_arFrame,
                                                              &arPointCloud);
        if (pointCloudStatus == AR_SUCCESS)
        {
            m_pointCloudRenderer.Draw(projectionMat * viewMat, m_arSession, arPointCloud);
            ArPointCloud_release(arPointCloud);
        }
    }

    bool RayArApplication::IsDepthSupported()
    {
        int32_t isSupported = 0;
        ArSession_isDepthModeSupported(m_arSession, AR_DEPTH_MODE_AUTOMATIC,&isSupported);
        return isSupported;
    }

    void RayArApplication::ConfigureSession()
    {
        const bool isDepthSupported = IsDepthSupported();

        ArConfig* arConfig = nullptr;
        ArConfig_create(m_arSession, &arConfig);
        ArConfig_setDepthMode(m_arSession, arConfig,
                              isDepthSupported ? AR_DEPTH_MODE_AUTOMATIC : AR_DEPTH_MODE_DISABLED);
        ArConfig_setInstantPlacementMode(m_arSession, arConfig,
                                         m_isInstantPlacementEnabled ? AR_INSTANT_PLACEMENT_MODE_LOCAL_Y_UP : AR_INSTANT_PLACEMENT_MODE_DISABLED);

        CHECK(arConfig);
        CHECK(ArSession_configure(m_arSession, arConfig) == AR_SUCCESS);
        ArConfig_destroy(arConfig);
    }

    void RayArApplication::OnSettingsChange(bool isInstantPlacementEnabled)
    {
        m_isInstantPlacementEnabled = isInstantPlacementEnabled;
        if (m_arSession != nullptr)
            ConfigureSession();
    }

    void RayArApplication::OnTouched(float x, float y)
    {
        if (m_arFrame == nullptr || m_arSession == nullptr) return;

        ArHitResultList* hitResultList = nullptr;
        ArHitResultList_create(m_arSession, &hitResultList);
        CHECK(hitResultList);
        if (m_isInstantPlacementEnabled)
        {
            ArFrame_hitTestInstantPlacement(m_arSession, m_arFrame, x, y,
                                            k_ApproximateDistanceMeters,
                                            hitResultList);
        } else
        {
            ArFrame_hitTest(m_arSession, m_arFrame, x, y, hitResultList);
        }

        int32_t hitResultListSize = 0;
        ArHitResultList_getSize(m_arSession, hitResultList,&hitResultListSize);

        // The hitTest method sorts the resulting list by distance from the camera,
        // increasing.  The first hit result will usually be the most relevant when
        // responding to user input.

        ArHitResult* arHitResult = nullptr;
        for (int32_t i = 0; i < hitResultListSize; ++i)
        {
            ArHitResult* arHit = nullptr;
            ArHitResult_create(m_arSession, &arHit);
            ArHitResultList_getItem(m_arSession, hitResultList, i, arHit);

            if (arHit == nullptr)
            {
                LOGE("RayArApplication::OnTouched ArHitResultList_getItem error");
                return;
            }

            ArTrackable* arTrackable = nullptr;
            ArHitResult_acquireTrackable(m_arSession, arHit, &arTrackable);
            ArTrackableType arTrackableType = AR_TRACKABLE_NOT_VALID;
            ArTrackable_getType(m_arSession, arTrackable, &arTrackableType);
            // Creates an Anchor if a plane or an oriented point was hit.
            if (arTrackableType == AR_TRACKABLE_PLANE)
            {
                ArPose* hitPose = nullptr;
                ArPose_create(m_arSession, nullptr, &hitPose);
                ArHitResult_getHitPose(m_arSession, arHit, hitPose);
                int32_t inPolygon = 0;
                ArPlane* arPlane = ArAsPlane(arTrackable);
                ArPlane_isPoseInPolygon(m_arSession, arPlane, hitPose, &inPolygon);

                // Use hit pose and camera pose to check if hittest is from the
                // back of the plane, if it is, no need to create the Anchor.
                ArPose* cameraPose = nullptr;
                ArPose_create(m_arSession, nullptr, &cameraPose);
                ArCamera* arCamera;
                ArFrame_acquireCamera(m_arSession, m_arFrame, &arCamera);
                ArCamera_getPose(m_arSession, arCamera, cameraPose);
                ArCamera_release(arCamera);
                float normalDistanceToPlane = Util::CalculateDistanceToPlane(*m_arSession,
                                                                             *hitPose, *cameraPose);

                ArPose_destroy(hitPose);
                ArPose_destroy(cameraPose);

                if (!inPolygon || normalDistanceToPlane < 0)
                    continue;

                arHitResult = arHit;
                break;
            } else if (arTrackableType == AR_TRACKABLE_POINT)
            {
                ArPoint* arPoint = ArAsPoint(arTrackable);
                ArPointOrientationMode mode;
                ArPoint_getOrientationMode(m_arSession, arPoint, &mode);
                if (mode == AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL)
                {
                    arHitResult = arHit;
                    break;
                }
            } else if (arTrackableType == AR_TRACKABLE_INSTANT_PLACEMENT_POINT)
            {
                arHitResult = arHit;
            } else if (arTrackableType == AR_TRACKABLE_DEPTH_POINT)
            {
                // ArDepthPoints are only returned if ArConfig_setDepthMode() is called
                // with AR_DEPTH_MODE_AUTOMATIC.
                arHitResult = arHit;
            }
        }

        if (arHitResult)
        {
            // Note that the application is responsible for releasing the Anchor
            // pointer after using it. Call ArAnchor_release(Anchor) to release.
            ArAnchor* anchor = nullptr;
            if (ArHitResult_acquireNewAnchor(m_arSession, arHitResult, &anchor) != AR_SUCCESS)
            {
                LOGE("RayArApplication::OnTouched ArHitResult_acquireNewAnchor error");
                return;
            }

            ArTrackingState arTrackingState = AR_TRACKING_STATE_STOPPED;
            ArAnchor_getTrackingState(m_arSession, anchor, &arTrackingState);
            if (arTrackingState != AR_TRACKING_STATE_TRACKING)
            {
                ArAnchor_release(anchor);
                return;
            }

            if (m_anchors.size() >= k_MaxNumberOfObjectsToRender)
            {
                ArAnchor_release(m_anchors[0].Anchor);
                ArTrackable_release(m_anchors[0].Trackable);
                m_anchors.erase(m_anchors.begin());
            }

            ArTrackable* trackable = nullptr;
            ArHitResult_acquireTrackable(m_arSession, arHitResult, &trackable);
            // Assign a Color to the object for rendering based on the Trackable type
            // this Anchor attached to. For AR_TRACKABLE_POINT, it's blue Color, and
            // for AR_TRACKABLE_PLANE, it's green Color.
            ColoredAnchor coloredAnchor{};
            coloredAnchor.Anchor = anchor;
            coloredAnchor.Trackable = trackable;

            UpdateAnchorColor(&coloredAnchor);
            m_anchors.push_back(coloredAnchor);

            ArHitResult_destroy(arHitResult);
            arHitResult = nullptr;

            ArHitResultList_destroy(hitResultList);
            hitResultList = nullptr;
        }
    }

    void RayArApplication::UpdateAnchorColor(ColoredAnchor* coloredAnchor)
    {
        ArTrackable* trackable = coloredAnchor->Trackable;
        float* color = coloredAnchor->Color;

        ArTrackableType trackableType;
        ArTrackable_getType(m_arSession, trackable, &trackableType);

        if (trackableType == AR_TRACKABLE_POINT)
        {
            SetColor(66.0f, 133.0f, 244.0f, 255.0f, color);
            return;
        }

        if (trackableType == AR_TRACKABLE_PLANE)
        {
            SetColor(139.0f, 195.0f, 74.0f, 255.0f, color);
            return;
        }

        if (trackableType == AR_TRACKABLE_DEPTH_POINT)
        {
            SetColor(199.0f, 8.0f, 65.0f, 255.0f, color);
            return;
        }

        if (trackableType == AR_TRACKABLE_INSTANT_PLACEMENT_POINT)
        {
            ArInstantPlacementPoint* arInstantPlacementPoint = ArAsInstantPlacementPoint(trackable);
            ArInstantPlacementPointTrackingMethod trackingMethod;
            ArInstantPlacementPoint_getTrackingMethod(m_arSession,
                                                      arInstantPlacementPoint,
                                                      &trackingMethod);
            if (trackingMethod == AR_INSTANT_PLACEMENT_POINT_TRACKING_METHOD_FULL_TRACKING)
            {
                SetColor(255.0f, 255.0f, 137.0f, 255.0f, color);
                return;
            } else if (trackingMethod == AR_INSTANT_PLACEMENT_POINT_TRACKING_METHOD_SCREENSPACE_WITH_APPROXIMATE_DISTANCE)
            {  // NOLINT
                SetColor(255.0f, 255.0f, 255.0f, 255.0f, color);
                return;
            }
        }

        // Fallback Color
        SetColor(0.0f, 0.0f, 0.0f, 0.0f, color);
    }

    // This method returns a transformation matrix that when applied to screen space
    // uvs makes them match correctly with the quad texture coords used to render
    // the camera feed. It takes into account device orientation.
    glm::mat3 RayArApplication::GetTextureTransformMatrix(const ArSession* session, const ArFrame* frame)
    {
        float frameTransform[6];
        float uvTransform[9];
        // XY pairs of coordinates in NDC space that constitute the origin and points
        // along the two principal axes.
        const float ndcBasis[6] = {0, 0, 1, 0, 0, 1};
        ArFrame_transformCoordinates2d(session, frame,
                                       AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES,
                                       3,ndcBasis,
                                       AR_COORDINATES_2D_TEXTURE_NORMALIZED,
                                       frameTransform);

        // Convert the transformed points into an affine transform and transpose it.
        float ndcOriginX = frameTransform[0];
        float ndcOriginY = frameTransform[1];
        uvTransform[0] = frameTransform[2] - ndcOriginX;
        uvTransform[1] = frameTransform[3] - ndcOriginY;
        uvTransform[2] = 0;
        uvTransform[3] = frameTransform[4] - ndcOriginX;
        uvTransform[4] = frameTransform[5] - ndcOriginY;
        uvTransform[5] = 0;
        uvTransform[6] = ndcOriginX;
        uvTransform[7] = ndcOriginY;
        uvTransform[8] = 1;

        return glm::make_mat3(uvTransform);
    }
}  // namespace Ray
