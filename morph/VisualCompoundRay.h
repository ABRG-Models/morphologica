/*
 * An extension of morph::Visual which contains code to output a glTF description of a scene that is
 * compatible with (i.e. can be opened by) compound-ray
 * (https://github.com/BrainsOnBoard/compound-ray)
 *
 * \author Seb James
 * \date July 2024
 */

#include <fstream>
#include <string>
#include <morph/Visual.h>
#include <morph/vec.h>

namespace morph {

    template <int glver = morph::gl::version_4_1>
    struct VisualCompoundRay : public morph::Visual<glver>
    {
        VisualCompoundRay (int width, int height, const std::string& title,
                           const morph::vec<float, 2> caOffset = { -0.8f, -0.8f },
                           const morph::vec<float> caLength = { 0.05f, 0.05f, 0.05f },
                           const float caThickness = 2.0f, const float caEm = 0.0f)
            : morph::Visual<glver> (width, height, title, caOffset, caLength, caThickness, caEm) {}
    public:
        //! If set true, then output additional glTF to make files compatible with compound-ray
        bool enable_compound_ray_gltf = true;

        //! Path to the compound eye file (this file is part of compound-ray, not morphologica)
        std::string path_to_compound_eye = "eyes/1000-horizontallyAcute-variableDegree.eye";

        //! We simply override the savegltf function to output in compound-ray format.
        virtual void savegltf (const std::string& gltf_file)
        {
            std::ofstream fout;
            fout.open (gltf_file, std::ios::out|std::ios::trunc);
            if (!fout.is_open()) {
                throw std::runtime_error ("VisualCompoundRay::savegltf(): Failed to open file for writing");
            }

            // Output the various sections of the gltf file
            this->gltf_scenes (fout);
            this->gltf_nodes (fout);
            this->gltf_cameras (fout);
            this->gltf_meshes (fout);
            this->gltf_buffers (fout);
            this->gltf_materials (fout);
            this->gltf_asset (fout);

            fout.close();
        }

    protected:
        //! Compound-ray gltf needs a background-shader to be specified. This is added to the
        //! "scenes" section
        virtual void compoundRayBackground (std::ofstream& fout) const
        {
            fout << "\"extras\" : { \"background-shader\": \"simple_sky\" }, ";
        }

        virtual void compoundRayPanCam (std::ofstream& fout) const
        {
            fout << "    {\n"
                 << "      \"name\" : \"regular-panoramic\",\n"
                 << "      \"type\" : \"perspective\",\n"
                 << "      \"perspective\" : {\n"
                 << "        \"aspectRatio\" : 1.7777777777777777,\n"
                 << "        \"yfov\" : 0.39959652046304894,\n"
                 << "        \"zfar\" : 1000,\n"
                 << "        \"znear\" : 0.10000000149011612\n"
                 << "      },\n"
                 << "      \"extras\" : {\n"
                 << "        \"panoramic\" : \"true\"\n"
                 << "      }\n"
                 << "    }";
        }

        virtual void compoundRayEyeCam (std::ofstream& fout) const
        {
            fout << "    {\n"
                 << "      \"name\" : \"simulated-compound-eye\",\n"
                 << "      \"type\" : \"perspective\",\n"
                 << "      \"perspective\" : {\n"
                 << "        \"aspectRatio\" : 1.7777777777777777,\n"
                 << "        \"yfov\" : 0.39959652046304894,\n"
                 << "        \"zfar\" : 1000,\n"
                 << "        \"znear\" : 0.10000000149011612\n"
                 << "      },\n"
                 << "      \"extras\" : {\n"
                 << "        \"compound-eye\" : \"true\",\n"
                 << "        \"compound-projection\" : \"spherical_orientationwise\",\n"
                 << "        \"compound-structure\" : \"" << this->path_to_compound_eye << "\"\n"
                 << "      }\n"
                 << "    }";
        }

        //! This outputs an example of a compound-ray compatible cameras section
        virtual void compoundRayCameras (std::ofstream& fout) const
        {
            fout << "  \"cameras\" : [\n";
            // Output camera sections of the cameras array
            this->compoundRayPanCam (fout);
            fout << ",\n";
            this->compoundRayEyeCam (fout);
            fout << "\n"
                 << "  ],\n";
        }

        //! Hardcoded camera nodes for compound-ray compatible gltf. This goes in the gltf "nodes"
        //! section.
        virtual void compoundRayCameraNodes (std::ofstream& fout) const
        {
            fout << "    {\n"
                 << "      \"camera\" : 0,\n"
                 << "      \"name\" : \"regular-panoramic_Orientation\",\n"
                 << "      \"rotation\" : [ -0.7071067690849304, 0, 0, 0.7071067690849304 ]\n"
                 << "    },\n"
                 << "    {\n"
                 << "      \"children\" : [ 0 ],\n"
                 << "      \"name\" : \"regular-panoramic\",\n"
                 << "      \"rotation\" : [ 0.7071068286895752, 0, 0, 0.7071068286895752 ]\n"
                 << "    },\n"
                 << "    {\n"
                 << "      \"camera\" : 1,\n"
                 << "      \"name\" : \"simulated-compound-eye_Orientation\",\n"
                 << "      \"rotation\" : [ -0.7071067690849304, 0, 0, 0.7071067690849304 ]\n"
                 << "    },\n"
                 << "    {\n"
                 << "      \"children\" : [ 2 ],\n"
                 << "      \"name\" : \"simulated-compound-eye\",\n"
                 << "      \"rotation\" : [ 0.7071068286895752, 0, 0, 0.7071068286895752 ]\n"
                 << "    },\n";
        }

        //! Output a scenes section of glTF
        virtual void gltf_scenes (std::ofstream& fout) const
        {
            fout << "{\n  \"scenes\" : [ { ";
            if (this->enable_compound_ray_gltf == true) { compoundRayBackground (fout); }
            fout << "\"nodes\" : [ ";
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                fout << vmi << (vmi < this->vm.size()-1 ? ", " : "");
            }
            fout << " ] } ],\n";
        }

        //! Output a nodes section of glTF
        virtual void gltf_nodes (std::ofstream& fout) const
        {
            fout << "  \"nodes\" : [\n";
            if (this->enable_compound_ray_gltf == true) { compoundRayCameraNodes (fout); }
            // for loop over VisualModels "mesh" : 0, etc
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                fout << "    { \"mesh\" : " << vmi
                     << ", \"translation\" : " << this->vm[vmi]->translation_str()
                     << (vmi < this->vm.size()-1 ? " },\n" : " }\n");
            }
            fout << "  ],\n";
        }

        //! Output a cameras section of glTF
        virtual void gltf_cameras (std::ofstream& fout) const
        {
            if (this->enable_compound_ray_gltf == true) { compoundRayCameras (fout); }
        }

        //! Output a meshes section of glTF
        virtual void gltf_meshes (std::ofstream& fout) const
        {
            // glTF meshes
            fout << "  \"meshes\" : [\n";
            // for each VisualModel:
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                fout << "    { \"primitives\" : [ { \"attributes\" : { \"POSITION\" : " << 1+vmi*4
                     << ", \"COLOR_0\" : " << 2+vmi*4
                     << ", \"NORMAL\" : " << 3+vmi*4 << " }, \"indices\" : " << vmi*4 << ", \"material\": 0 } ] }"
                     << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";
        }

        // Output the buffers, bufferviews and accessors sections of glTF
        virtual void gltf_buffers (std::ofstream& fout) const
        {
            // glTF buffers
            fout << "  \"buffers\" : [\n";
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                // indices
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->indices_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->indices_bytes() << "},\n";
                // pos
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->vpos_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->vpos_bytes() << "},\n";
                // col
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->vcol_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->vcol_bytes() << "},\n";
                // norm
                fout << "    {\"uri\" : \"data:application/octet-stream;base64," << this->vm[vmi]->vnorm_base64() << "\", "
                     << "\"byteLength\" : " << this->vm[vmi]->vnorm_bytes() << "}";
                fout << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";

            // glTF bufferViews
            fout << "  \"bufferViews\" : [\n";
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                // indices
                fout << "    { ";
                fout << "\"buffer\" : " << vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->indices_bytes() << ", ";
                fout << "\"target\" : 34963 ";
                fout << " },\n";
                // vpos
                fout << "    { ";
                fout << "\"buffer\" : " << 1+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->vpos_bytes() << ", ";
                fout << "\"target\" : 34962 ";
                fout << " },\n";
                // vcol
                fout << "    { ";
                fout << "\"buffer\" : " << 2+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->vcol_bytes() << ", ";
                fout << "\"target\" : 34962 ";
                fout << " },\n";
                // vnorm
                fout << "    { ";
                fout << "\"buffer\" : " << 3+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"byteLength\" : " << this->vm[vmi]->vnorm_bytes() << ", ";
                fout << "\"target\" : 34962 ";
                fout << " }";
                fout << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";

            // glTF accessors
            fout << "  \"accessors\" : [\n";
            for (std::size_t vmi = 0u; vmi < this->vm.size(); ++vmi) {
                this->vm[vmi]->computeVertexMaxMins();
                // indices
                fout << "    { ";
                fout << "\"bufferView\" : " << vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                // 5123 unsigned short, 5121 unsigned byte, 5125 unsigned int, 5126 float:
                fout << "\"componentType\" : 5125, ";
                fout << "\"type\" : \"SCALAR\", ";
                fout << "\"count\" : " << this->vm[vmi]->indices_size();
                fout << "},\n";
                // vpos
                fout << "    { ";
                fout << "\"bufferView\" : " << 1+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"componentType\" : 5126, ";
                fout << "\"type\" : \"VEC3\", ";
                fout << "\"count\" : " << this->vm[vmi]->vpos_size()/3;
                // vertex position requires max/min to be specified in the gltf format
                fout << ", \"max\" : " << this->vm[vmi]->vpos_max() << ", ";
                fout << "\"min\" : " << this->vm[vmi]->vpos_min();
                fout << " },\n";
                // vcol
                fout << "    { ";
                fout << "\"bufferView\" : " << 2+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"componentType\" : 5126, ";
                fout << "\"type\" : \"VEC3\", ";
                fout << "\"count\" : " << this->vm[vmi]->vcol_size()/3;
                fout << "},\n";
                // vnorm
                fout << "    { ";
                fout << "\"bufferView\" : " << 3+vmi*4 << ", ";
                fout << "\"byteOffset\" : 0, ";
                fout << "\"componentType\" : 5126, ";
                fout << "\"type\" : \"VEC3\", ";
                fout << "\"count\" : " << this->vm[vmi]->vnorm_size()/3;
                fout << "}";
                fout << (vmi < this->vm.size()-1 ? ",\n" : "\n");
            }
            fout << "  ],\n";
        }

        //! Output a materials section of glTF
        virtual void gltf_materials (std::ofstream& fout) const
        {
            // Default material is single sided, so make it double sided
            fout << "  \"materials\" : [ { \"doubleSided\" : true } ],\n";
        }

        //! Output the asset section of glTF
        virtual void gltf_asset (std::ofstream& fout) const
        {
            fout << "  \"asset\" : {\n"
                 << "    \"generator\" : \"https://github.com/ABRG-Models/morphologica [version "
                 << morph::version_string() << "]: morph::VisualCompoundRay::savegltf()\",\n"
                 << "    \"version\" : \"2.0\"\n" // This version is the *glTF* version.
                 << "  }\n";
            fout << "}\n";
        }

    };

} // namespace
