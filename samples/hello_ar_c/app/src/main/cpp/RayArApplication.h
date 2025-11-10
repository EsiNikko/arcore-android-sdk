#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <jni.h>

#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "arcore_c_api.h"
#include "BackgroundRenderer.h"
#include "glm.h"
#include "ObjRenderer.h"
#include "PlaneRenderer.h"
#include "PointCloudRenderer.h"
#include "Texture.h"
#include "Util.h"

namespace Ray
{
    // RayArApplication handles all application logics.
    class RayArApplication
    {
    public:
        // Constructor and deconstructor.
        explicit RayArApplication(AAssetManager* assetManager);
        ~RayArApplication();

        // OnPause is called on the UI thread from the Activity's onPause method.
        void OnPause();
        // OnResume is called on the UI thread from the Activity's onResume method.
        void OnResume(JNIEnv* env, void* context, void* activity);
        // OnSurfaceCreated is called on the OpenGL thread when GLSurfaceView
        // is created.
        void OnSurfaceCreated();
        // OnDisplayGeometryChanged is called on the OpenGL thread when the
        // render surface size or display rotation changes.
        //
        // @param displayRotation: current display rotation.
        // @param width: width of the changed surface view.
        // @param height: height of the changed surface view.
        void OnDisplayGeometryChanged(int displayRotation, int width, int height);
        // OnDrawFrame is called on the OpenGL thread to render the next frame.
        void OnDrawFrame(bool depthColorVisualizationEnabled, bool useDepthForOcclusion);
        // OnTouched is called on the OpenGL thread after the user touches the screen.
        // @param x: x position on the screen (pixels).
        // @param y: y position on the screen (pixels).
        void OnTouched(float x, float y);
        // Returns true if any planes have been detected.  Used for hiding the
        // "searching for planes" snackbar.
        bool HasDetectedPlanes() const { return m_detectedPlanesCount > 0; }
        // Returns true if depth is supported.
        bool IsDepthSupported();
        void OnSettingsChange(bool isInstantPlacementEnabled);

    private:
        // The anchors at which we are drawing android models using given colors.
        struct ColoredAnchor
        {
            ArAnchor* Anchor;
            ArTrackable* Trackable;
            float Color[4];
        };
        glm::mat3 GetTextureTransformMatrix(const ArSession* session, const ArFrame* frame);
        void ConfigureSession();
        void UpdateAnchorColor(ColoredAnchor* coloredAnchor);

        ArSession* m_arSession = nullptr;
        ArFrame* m_arFrame = nullptr;

        bool m_installRequested = false;
        bool m_calculateUvTransform = false;
        int m_width = 1;
        int m_height = 1;
        int m_displayRotation = 0;
        bool m_isInstantPlacementEnabled = true;

        AAssetManager* const m_assetManager = nullptr;
        std::vector<ColoredAnchor> m_anchors;
        PointCloudRenderer m_pointCloudRenderer;
        BackgroundRenderer m_backgroundRenderer;
        PlaneRenderer m_planeRenderer;
        ObjRenderer m_objRenderer;
        Texture m_depthTexture;

        int32_t m_detectedPlanesCount = 0;
    };
}  // namespace Ray
