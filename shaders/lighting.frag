// #version 410 core

// output shader variable
out vec4 colorFrag;

uniform uint nPointLights;
uniform uint nDirLights;
uniform uint nSpotLights;

uniform 	PointLight 		    pointLights[MAX_POINT_LIGHTS];
in 		    LightIncidence 	    pointLI[MAX_POINT_LIGHTS];

uniform 	DirectionalLight 	directionalLights[MAX_DIR_LIGHTS];
in 		    LightIncidence 	    dirLI[MAX_DIR_LIGHTS];

uniform 	SpotLight 			spotLights[MAX_SPOT_LIGHTS];
in 		    LightIncidence 	    spotLI[MAX_SPOT_LIGHTS];

// parameters for current light calc
LightAttributes currLA;
LightIncidence  currLI;

// shininess coefficients (passed from the application)
uniform float shininess;

// uniforms for GGX model
uniform float alpha; // rugosity - 0 : smooth, 1: rough
uniform float F0; // fresnel reflectance at normal incidence

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec3 illum_model();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform illum_model Illumination_Model;

////////////////////////////////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Lambert model
subroutine(illum_model)
vec3 Lambert() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // normalization of the per-fragment normal
    vec3 N = normalize(currLI.vNormal);
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(currLI.lightDir);

    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);

    // Lambert illumination model
    return vec3(currLA.kD * lambertian * currLA.diffuse);
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Phong model
subroutine(illum_model)
vec3 Phong() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // ambient component can be calculated at the beginning
    vec3 color = currLA.kA * currLA.ambient;

    // normalization of the per-fragment normal
    vec3 N = normalize(currLI.vNormal);

    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(currLI.lightDir);

    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);

    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0)
    {
      // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
      vec3 V = normalize( currLI.vViewPosition );

      // reflection vector
      vec3 R = reflect(-L, N);

      // cosine of angle between R and V
      float specAngle = max(dot(R, V), 0.0);
      // shininess application to the specular component
      float specular = pow(specAngle, shininess);

      // We add diffusive and specular components to the final color
      // N.B. ): in this implementation, the sum of the components can be different than 1
      color += vec3( currLA.kD * lambertian * currLA.diffuse +
                     currLA.kS * specular   * currLA.specular);
    }
    return color;
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the Blinn-Phong model
subroutine(illum_model)
vec3 BlinnPhong() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // ambient component can be calculated at the beginning
    vec3 color = currLA.kA * currLA.ambient;

    // normalization of the per-fragment normal
    vec3 N = normalize(currLI.vNormal);

    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(currLI.lightDir);

    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);

    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0)
    {
      // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
      vec3 V = normalize( currLI.vViewPosition );

      // in the Blinn-Phong model we do not use the reflection vector, but the half vector
      vec3 H = normalize(L + V);

      // we use H to calculate the specular component
      float specAngle = max(dot(H, N), 0.0);
      // shininess application to the specular component
      float specular = pow(specAngle, shininess);

      // We add diffusive and specular components to the final color
      // N.B. ): in this implementation, the sum of the components can be different than 1
      color += vec3( currLA.kD * lambertian * currLA.diffuse +
                     currLA.kS * specular   * currLA.specular);
    }
    return color;
}
//////////////////////////////////////////

//////////////////////////////////////////
// Schlick-GGX method for geometry obstruction (used by GGX model)
float G1(float angle, float alpha)
{
    // in case of Image Based Lighting, the k factor is different:
    // usually it is set as k=(alpha*alpha)/2
    float r = (alpha + 1.0);
    float k = (r*r) / 8.0;

    float num   = angle;
    float denom = angle * (1.0 - k) + k;

    return num / denom;
}

//////////////////////////////////////////
// a subroutine for the GGX model
subroutine(illum_model)
vec3 GGX() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // normalization of the per-fragment normal
    vec3 N = normalize(currLI.vNormal);
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(currLI.lightDir);

    // cosine angle between direction of light and normal
    float NdotL = max(dot(N, L), 0.0);

    // diffusive (Lambert) reflection component
    vec3 lambert = (currLA.kD * currLA.diffuse)/PI;

    // we initialize the specular component
    vec3 specular = vec3(0.0);

    // if the cosine of the angle between direction of light and normal is positive, then I can calculate the specular component
    if(NdotL > 0.0)
    {
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( currLI.vViewPosition );

        // half vector
        vec3 H = normalize(L + V);

        // we implement the components seen in the slides for a PBR BRDF
        // we calculate the cosines and parameters to be used in the different components
        float NdotH = max(dot(N, H), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float VdotH = max(dot(V, H), 0.0);
        float alpha_Squared = alpha * alpha;
        float NdotH_Squared = NdotH * NdotH;

        // Geometric factor G2
        // Smith???s method (uses Schlick-GGX method for both geometry obstruction and shadowing )
        float G2 = G1(NdotV, alpha)*G1(NdotL, alpha);

        // Rugosity D
        // GGX Distribution
        float D = alpha_Squared;
        float denom = (NdotH_Squared*(alpha_Squared-1.0)+1.0);
        D /= 1;//PI*denom*denom;

        // Fresnel reflectance F (approx Schlick)
        vec3 F = vec3(pow(1.0 - VdotH, 5.0));
        F *= (1.0 - F0);
        F += F0;

        // we put everything together for the specular component
        specular = (F * G2 * D) / (4.0 * NdotV * NdotL);
    }

    // the rendering equation is:
    // integral of: BRDF * Li * (cosine angle between N and L)
    // BRDF in our case is: the sum of Lambert and GGX
    // Li is considered as equal to 1: light is white, and we have not applied attenuation. With colored lights, and with attenuation, the code must be modified and the Li factor must be multiplied to finalColor
    return (lambert + specular)*NdotL;
}
//////////////////////////////////////////

vec3 calcPointLights()
{
    vec3 color = vec3(0);

    for(int i = 0; i < nPointLights; i++)
    {
        currLA = pointLights[i].lightAttrs;
        currLI = pointLI[i];
        color += Illumination_Model();
    }

    return color;
}

vec3 calcDirLights()
{
    vec3 color = vec3(0);

    // TODO

    return color;
}

vec3 calcSpotLights()
{
    vec3 color = vec3(0);

    // TODO

    return color;
}

// main
void main(void)
{
    // we call the pointer function Illumination_Model():
    // the subroutine selected in the main application will be called and executed
    vec3 color = vec3(0);
    
    color += calcPointLights();
    color += calcDirLights();
    color += calcSpotLights();
      
    
    //vec3 color = Illumination_Model();

    colorFrag = vec4(color, 1.0);
}