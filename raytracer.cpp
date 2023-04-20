/****************************************************
 * 2016-2023 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/
#include "raytracer.h"
#include "ishape.h"
#include "io.h"

 /**
  * @fn	RayTracer::RayTracer(const color &defa)
  * @brief	Constructs a raytracers.
  * @param	defa	The clear color.
  */

RayTracer::RayTracer(const color& defa)
	: defaultColor(defa) {
}

/**
 * @fn	void RayTracer::raytraceScene(FrameBuffer &frameBuffer, int depth, const IScene &theScene) const
 * @brief	Raytrace scene
 * @param [in,out]	frameBuffer	Framebuffer.
 * @param 		  	depth	   	The current depth of recursion.
 * @param 		  	theScene   	The scene.
 */

 void RayTracer::raytraceScene(FrameBuffer& frameBuffer, int depth,
 	const IScene& theScene, int N) const {
 	const RaytracingCamera& camera = *theScene.camera;
 	const vector<VisibleIShapePtr>& objs = theScene.opaqueObjs;
    const vector<TransparentIShapePtr>& transObjs = theScene.transparentObjs;
 	const vector<LightSourcePtr>& lights = theScene.lights;
 	color defaultColor = frameBuffer.getClearColor();
 
 	for (int y = 0; y < frameBuffer.getWindowHeight(); ++y) {
 		for (int x = 0; x < frameBuffer.getWindowWidth(); ++x) {
 			DEBUG_PIXEL = (x == xDebug && y == yDebug);
 			if (DEBUG_PIXEL) {
 				cout << "";
 			}

 			color sum = dvec3(0, 0, 0);
 			double factor = 1.0 / N;
 			double pixelX = x + (1.0 / (2 * N));
 			double pixelY = y + (1.0 / (2 * N));
 			int iterations = 1;

 			for (int i = 0; i < N; i++) {
 				if (i != 0) {
 					pixelX += factor;
 				}

 				pixelY = y + 1.0 / (2 * N);
 				for (int j = 0; j < N; j++) {

 					Ray ray = camera.getRay(pixelX, pixelY);
 					if (N == 1) ray = camera.getRay(x, y);

 					OpaqueHitRecord hit;
 					VisibleIShape::findIntersection(ray, objs, hit);

                    TransparentHitRecord transHit;
                    TransparentIShape::findIntersection(ray, transObjs, transHit);

 					if (hit.t != FLT_MAX && transHit.t == FLT_MAX) {

 						if (glm::dot(ray.dir, hit.normal) > 0) {
 							hit.normal = -hit.normal;
 						}

 						if (hit.texture != nullptr) {
 							color texel = hit.texture->getPixelUV(hit.u, hit.v);
 							frameBuffer.setColor(x, y, texel);
 						} else {
                            for (int k = 0; k < lights.size(); k++) {
                                sum += lights[k]->illuminate(hit.interceptPt, hit.normal, hit.material, camera.getFrame(), lights[k]->pointIsInAShadow(hit.interceptPt, hit.normal, objs, camera.getFrame()));
                            }
                            if (iterations == (N * N) || N == 1) {
 								dvec3 final(N * N, N * N, N * N);
 								frameBuffer.setColor(x, y, sum / final);
 							}
 						}
                    } else if (hit.t == FLT_MAX && transHit.t != FLT_MAX) {
                        for (int k = 0; k < lights.size(); k++) {
                            sum += transHit.transColor;
                        }
                        if (iterations == (N * N) || N == 1) {
                            dvec3 final(N * N, N * N, N * N);
                            frameBuffer.setColor(x, y, sum / final);
                        }

                    } else if (hit.t != FLT_MAX && transHit.t != FLT_MAX) {
                        for (int k = 0; k < lights.size(); k++) {
                            sum += lights[k]->illuminate(hit.interceptPt, hit.normal, hit.material, camera.getFrame(), lights[k]->pointIsInAShadow(hit.interceptPt, hit.normal, objs, camera.getFrame()));
                        }
                        if (hit.t < transHit.t) {
                            if (iterations == (N * N) || N == 1) {
                                dvec3 final(N * N, N * N, N * N);
                                frameBuffer.setColor(x, y, sum / final);
                            }
                        } else {
                            if (iterations == (N * N) || N == 1) {
                                dvec3 final(N * N, N * N, N * N);
                                color dest = sum / final;
                                color source = transHit.transColor;
                                frameBuffer.setColor(x, y, ((1 - transHit.alpha) * dest) + (transHit.alpha * source));
                            }
                        }
                    }
 					frameBuffer.showAxes(x, y, ray, 0.25);			// Displays R/x, G/y, B/z axes
 					pixelY += factor;
 					iterations++;
 				}
 			}
 		}
 	}
 	frameBuffer.showColorBuffer();
 }

//void RayTracer::raytraceScene(FrameBuffer& frameBuffer, int depth,
//	const IScene& theScene, int N) const {
//	const RaytracingCamera& camera = *theScene.camera;
//	const vector<VisibleIShapePtr>& objs = theScene.opaqueObjs;
//	const vector<LightSourcePtr>& lights = theScene.lights;
//	color defaultColor = frameBuffer.getClearColor();
//
//	for (int y = 0; y < frameBuffer.getWindowHeight(); ++y) {
//		for (int x = 0; x < frameBuffer.getWindowWidth(); ++x) {
//			DEBUG_PIXEL = (x == xDebug && y == yDebug);
//			if (DEBUG_PIXEL) {
//				cout << "";
//			}
//			Ray ray = camera.getRay(x, y);
//			OpaqueHitRecord hit;
//			VisibleIShape::findIntersection(ray, objs, hit);
//			if (hit.t != FLT_MAX) {
//				color C = lights[0]->illuminate(hit.interceptPt, hit.normal, hit.material, camera.getFrame(), lights[0]->pointIsInAShadow(hit.interceptPt, hit.normal, objs, camera.getFrame()));;
//				frameBuffer.setColor(x, y, C);
//			}
//			frameBuffer.showAxes(x, y, ray, 0.25);			// Displays R/x, G/y, B/z axes
//		}
//	}
//
//	frameBuffer.showColorBuffer();
//}

//void RayTracer::raytraceScene(FrameBuffer& frameBuffer, int depth,
//	const IScene& theScene, int N) const {
//	const RaytracingCamera& camera = *theScene.camera;
//	const vector<VisibleIShapePtr>& objs = theScene.opaqueObjs;
//	const vector<LightSourcePtr>& lights = theScene.lights;
//	color defaultColor = frameBuffer.getClearColor();
//
//	for (int y = 0; y < frameBuffer.getWindowHeight(); ++y) {
//		for (int x = 0; x < frameBuffer.getWindowWidth(); ++x) {
//			DEBUG_PIXEL = (x == xDebug && y == yDebug);
//			if (DEBUG_PIXEL) {
//				cout << "";
//			}
//			color sum = dvec3(0, 0, 0);
//			double factor = 1.0 / N;
//			double pixelX = x + (1.0 / (2 * N));
//			double pixelY = y + (1.0 / (2 * N));
//			int iterations = 1;
//			for (int i = 0; i < N; i++) {
//				if (i != 0) {
//					pixelX += factor;
//				}
//				pixelY = y + 1.0 / (2 * N);
//				for (int j = 0; j < N; j++) {
//					Ray ray = camera.getRay(pixelX, pixelY);
//					if (N == 1) ray = camera.getRay(x, y);
//					OpaqueHitRecord hit;
//					VisibleIShape::findIntersection(ray, objs, hit);
//					if (hit.t != FLT_MAX) {
//						if (glm::dot(ray.dir, hit.normal) > 0) {
//							hit.normal = -hit.normal;
//						}
//						if (hit.texture != nullptr) {
//							color texel = hit.texture->getPixelUV(hit.u, hit.v);
//							frameBuffer.setColor(x, y, texel);
//						}
//						else {
//							for (int k = 0; k < lights.size(); k++) {
//								sum += lights[k]->illuminate(hit.interceptPt, hit.normal, hit.material, camera.getFrame(), lights[k]->pointIsInAShadow(hit.interceptPt, hit.normal, objs, camera.getFrame()));
//								if (iterations == (N * N) || N == 1) {
//									dvec3 final(N * N, N * N, N * N);
//									frameBuffer.setColor(x, y, sum / final);
//								}
//							}
//						}
//					}
//					frameBuffer.showAxes(x, y, ray, 0.25);			// Displays R/x, G/y, B/z axes
//					pixelY += factor;
//					iterations++;
//				}
//			}
//		}
//	}
//	frameBuffer.showColorBuffer();
//}

//void RayTracer::raytraceScene(FrameBuffer& frameBuffer, int depth,
//	const IScene& theScene, int N) const {
//	const RaytracingCamera& camera = *theScene.camera;
//	const vector<VisibleIShapePtr>& objs = theScene.opaqueObjs;
//	const vector<TransparentIShapePtr>& transObjs = theScene.transparentObjs;
//	const vector<LightSourcePtr>& lights = theScene.lights;
//	color defaultColor = frameBuffer.getClearColor();
//
//	for (int y = 0; y < frameBuffer.getWindowHeight(); ++y) {
//		for (int x = 0; x < frameBuffer.getWindowWidth(); ++x) {
//			DEBUG_PIXEL = (x == xDebug && y == yDebug);
//			if (DEBUG_PIXEL) {
//				cout << "";
//			}
//			color sum = dvec3(0, 0, 0);
//			double factor = 1.0 / N;
//			double pixelX = x + (1.0 / (2 * N));
//			double pixelY = y + (1.0 / (2 * N));
//			int iterations = 1;
//			for (int i = 0; i < N; i++) {
//				if (i != 0) {
//					pixelX += factor;
//				}
//				pixelY = y + 1.0 / (2 * N);
//				for (int j = 0; j < N; j++) {
//					Ray ray = camera.getRay(pixelX, pixelY);
//					if (N == 1) ray = camera.getRay(x, y);
//					OpaqueHitRecord hit;
//					VisibleIShape::findIntersection(ray, objs, hit);
//					TransparentHitRecord transHit;
//					TransparentIShape::findIntersection(ray, transObjs, transHit);
//					if (hit.t != FLT_MAX && transHit.t == FLT_MAX) {
//						if (glm::dot(ray.dir, hit.normal) > 0) {
//							hit.normal = -hit.normal;
//						}
//						if (hit.texture != nullptr) {
//							color texel = hit.texture->getPixelUV(hit.u, hit.v);
//							frameBuffer.setColor(x, y, texel);
//						} else {
//							for (int k = 0; k < lights.size(); k++) {
//								sum += lights[k]->illuminate(hit.interceptPt, hit.normal, hit.material, camera.getFrame(), lights[k]->pointIsInAShadow(hit.interceptPt, hit.normal, objs, camera.getFrame()));
//							}
//							if (iterations == (N * N) || N == 1) {
//								dvec3 final(N * N, N * N, N * N);
//								frameBuffer.setColor(x, y, sum / final);
//							}
//						}
//					} else if (hit.t == FLT_MAX && transHit.t != FLT_MAX) {
//						for (int k = 0; k < lights.size(); k++) {
//							sum += transHit.transColor;
//						}
//						if (iterations == (N * N) || N == 1) {
//							dvec3 final(N * N, N * N, N * N);
//							sum += defaultColor;
//							frameBuffer.setColor(x, y, sum / final);
//						}
//					} else if (hit.t != FLT_MAX && transHit.t != FLT_MAX) {
//						for (int k = 0; k < lights.size(); k++) {
//							sum += lights[k]->illuminate(hit.interceptPt, hit.normal, hit.material, camera.getFrame(), lights[k]->pointIsInAShadow(hit.interceptPt, hit.normal, objs, camera.getFrame()));
//						}
//						if (hit.t < transHit.t) {
//							if (iterations == (N * N) || N == 1) {
//								dvec3 final(N * N, N * N, N * N);
//								sum += defaultColor;
//								frameBuffer.setColor(x, y, sum / final);
//							}
//						} else {
//							if (iterations == (N * N) || N == 1) {
//								dvec3 final(N * N, N * N, N * N);
//								color dest = sum / final;
//								color source = transHit.transColor;
//								frameBuffer.setColor(x, y, ((1 - transHit.alpha) * dest ) + (transHit.alpha * transHit.transColor));
//							}
//						}
//					} else {
//						frameBuffer.setColor(x, y, defaultColor);
//					}
//					frameBuffer.showAxes(x, y, ray, 0.25);			// Displays R/x, G/y, B/z axes
//					pixelY += factor;
//					if ((iterations == (N * N) || N == 1) && sum == dvec3(0, 0, 0)) frameBuffer.setColor(x, y, defaultColor);
//					iterations++;
//				}
//			}
//		}
//	}
//	frameBuffer.showColorBuffer();
//}