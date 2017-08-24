/*
  Q Light Controller Plus
  LightPassEffect.qml

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

import Qt3D.Core 2.0
import Qt3D.Render 2.0

Effect
{
    techniques:
    [
        // OpenGL 3.1
        Technique
        {
            graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
            renderPasses:
                RenderPass
                {
                    filterKeys: FilterKey { name : "pass"; value : "final" }
                    shaderProgram: ShaderProgram {
                        id: finalShaderGL3
                        vertexShaderCode: loadSource("qrc:/final_gl3.vert")
                        fragmentShaderCode: loadSource("qrc:/final_gl3.frag")
                    }
                }
        },
        // OpenGL 2.0 with FBO extension
        Technique
        {
            graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.NoProfile; majorVersion: 2; minorVersion: 0 }
            renderPasses:
                RenderPass
                {
                    filterKeys: FilterKey { name: "pass"; value: "final" }
                    shaderProgram: ShaderProgram {
                        id: finalShaderGL2
                        vertexShaderCode: loadSource("qrc:/final_es2.vert")
                        fragmentShaderCode: loadSource("qrc:/final_es2.frag")
                    }
                }
        }
    ]
}
