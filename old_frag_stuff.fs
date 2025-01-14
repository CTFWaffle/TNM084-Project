
        
        if (normalizedHeight <= 0.99)
        {
            // Water area => sample rock or water texture
            // If you have a water texture, use that. 
            // Example using "rock" as placeholder:
            //color = texture(u_rockTex, uv).rgb;

            // float flowIntensity = noiser(fbm(modelPosition).xy * 10, u_time);
            //float flowIntensity = noiser(fbm(modelPosition).xy * 10, 0.1);
            //float flowIntensity = pnoise3(modelPosition.x * 100, int(modelPosition.y * 100), int(modelPosition.z * 100) * 0.1);
            float flowIntensity = noiser(fbm(modelPosition) * 10, 0.1);
            vec2 curl = computeCurl(modelPosition.xy * flowIntensity * 15.0); // Add time for animation
            float curlIntensity = length(curl); // Use curl magnitude to create swirls
            vec3 curlColor = vec3(0.2, 0.2, 0.5) * curlIntensity * 0.5; // Modulated color based on curl


            color = waterColor + curlColor; 
        }
        else if (normalizedHeight < 1.0)
        {
            // Sand
            // float n = 0.0f;
            // float freq = 30.0f; // Higher frequency for finer details
            // float amp = 1.0f;
            // for (int oct = 0; oct < 5; ++oct) {
            //     n += pnoise3(modelPosition.x * freq, int(modelPosition.y * freq), int(modelPosition.z * freq) * amp); // Use absolute noise for turbulence
            //     freq *= 2.0f; // Double the frequency for each octave
            //     amp *= 0.5f;  // Halve the amplitude for each octave
            // }
            // n = (n * 0.5f) + 0.5f; // Normalize to [0..1]

            // // Base sand color (light tan)
            // float baseR = 0.9f;
            // float baseG = 0.85f;
            // float baseB = 0.7f;

            // // Add noise-based shading to simulate subtle lumps
            // float shading = 0.9f + 0.1f * n; // Subtle shading variation
            // float r = baseR * shading;
            // float g = baseG * shading;
            // float b = baseB * shading;

            // // High-frequency noise for fine grain
            // float grain = pnoise3(modelPosition.x * 10.0, int(modelPosition.y * 10.0), int(modelPosition.z * 10.0));
            // grain = (grain * 0.5f) + 0.5f;
            // if (grain > 0.5f) {
            //     r *= 0.95f; // Slightly darken some grains
            //     g *= 0.95f;
            //     b *= 0.95f;
            // }
            // color = vec3(clamp(r, 0.0, 1.0), clamp(g, 0.0, 1.0), clamp(b, 0.0, 1.0));
            float flowIntensity = noiser(fbm(modelPosition) * 10, 0.1);
            vec2 curl = computeCurl(modelPosition.xy * flowIntensity * 15.0); // Add time for animation
            float curlIntensity = length(curl); // Use curl magnitude to create swirls
            vec3 curlColor = vec3(0.55, 0.55, 0.55) * curlIntensity * 0.5; // Modulated color based on curl


            //color = sandColor + curlColor; 
            
            color = mix(waterColor,
                        sandColor + curlColor,
                        t);
        }
        else if (normalizedHeight < 1.04)
        {
            // Grass
            // float n = 0.0f;
            // float freq = 160.0f;
            // float amp = 0.7f;
            // for (int oct = 0; oct < 5; ++oct)
            // {
            //     n += pnoise3(modelPosition.x * freq, int(modelPosition.y * freq), int(modelPosition.z * freq)) * amp;
            //     freq *= 2.0f;
            //     amp  *= 0.5f;
            // }
            // // n is roughly in [-1, 1], depending on noise function
            // // Shift and scale to [0, 1]
            // n = (n * 0.3f) + 0.7f;

            // // We'll make the base color green, vary brightness with noise
            // // We can also modulate hue slightly for patchy look
            // float baseR = 0.2f;
            // float baseG = 0.2f + 0.2f * n; // range ~ [0.6..0.8]
            // float baseB = 0.2f;
            // // Another noise to simulate small yellowish spots
            // float smallSpots = pnoise3(modelPosition.x * 40.0, int(modelPosition.y * 40.0), int(modelPosition.z * 40.0));
            // float patches = 0.1f + 0.9f * smallSpots; // [0..1]

            // // Mix in a bit of browny/ yellowy color in patches
            // float r = (1.0f - patches) * baseR + patches * 0.45f;
            // float g = (1.0f - patches) * baseG + patches * 0.45f;
            // float b = (1.0f - patches) * baseB + patches * 0.1f;

            // // Final brightness mod
            // float brightness = 0.8f + 0.2f * n;
            // r *= brightness;
            // g *= brightness;
            // b *= brightness;

            // color = vec3(clamp(r, 0.0, 1.0), clamp(g, 0.0, 1.0), clamp(b, 0.0, 1.0));
            float flowIntensity = noiser(fbm(modelPosition) * 10, 0.1);
            vec2 curl = computeCurl(modelPosition.xy * flowIntensity * 15.0); // Add time for animation
            float curlIntensity = length(curl); // Use curl magnitude to create swirls
            vec3 curlColor = vec3(0.45, 0.45, 0.1) * curlIntensity * 0.5; // Modulated color based on curl


            //color = grassColor + curlColor; 
            
            color = mix(sandColor,
                        grassColor + curlColor,
                        t);
        }
        else if (normalizedHeight < 1.07)//0.45)
        {

            // Multi-octave noise, but let's vary the frequency more
            // float n = 0.0;
            // float freq = 100.0;
            // float amp = 1.0;
            // for (int oct = 0; oct < 6; ++oct) {
            //     n += pnoise3(modelPosition.x * freq, int(modelPosition.y * freq), int(modelPosition.z * freq)) * amp;
            //     freq *= 2.0;
            //     amp *= 0.5;
            // }
            // n = (n * 0.5) + 0.5; // Shift to [0..1]

            // // Rocky color with variation
            // float r = 0.3 + 0.3 * n;
            // float g = 0.25 + 0.35 * n;
            // float b = 0.2 + 0.2 * n;

            // // Additional noise for speckles
            // float spots = pnoise3(modelPosition.x * 40.0, int(modelPosition.y * 40.0), int(modelPosition.z * 40.0));
            // spots = (spots * 0.5) + 0.5;
            // if (spots > 0.7) {
            //     r += 0.05;
            //     g += 0.02;
            // }

            // color = vec3(clamp(r, 0.0, 1.0), clamp(g, 0.0, 1.0), clamp(b, 0.0, 1.0));
            float flowIntensity = noiser(fbm(modelPosition) * 10, 0.1);
            vec2 curl = computeCurl(modelPosition.xy * flowIntensity * 15.0); // Add time for animation
            float curlIntensity = length(curl); // Use curl magnitude to create swirls
            vec3 curlColor = vec3(0.05, 0.02, 0) * curlIntensity * 0.5; // Modulated color based on curl


            // color = mountainColor + curlColor;
            
            color = mix(grassColor,
                        mountainColor + curlColor,
                        t);
        }
        else
        {
            // Snow
            // Use some fractal layering of Perlin noise
            // float n = 0.0f;
            // float freq = 100.0f;
            // float amp = 1.0f;
            // for (int oct = 0; oct < 5; ++oct)
            // {
            //     n += pnoise3(modelPosition.x * freq, int(modelPosition.y * freq), int(modelPosition.z * freq)) * amp;
            //     freq *= 2.0f;
            //     amp  *= 0.5f;
            // }
            
            // // For "snow," we want a mostly white, slightly bluish or grayish pattern
            // // We can make it whiter in the highlights, or do subtle shading
            // // Let’s compress the range so it’s mostly bright:
            // float brightness = 0.9f + 0.1f * n; // in [0.8..1.0] approx
            // float r = brightness;
            // float g = brightness * 0.98f; // slight bluish tint
            // float b = brightness * 0.99f;

            // color = vec3(clamp(r, 0.0, 1.0), clamp(g, 0.0, 1.0), clamp(b, 0.0, 1.0));
            float flowIntensity = noiser(fbm(modelPosition) * 10, 0.1);
            vec2 curl = computeCurl(modelPosition.xy * flowIntensity * 15.0); // Add time for animation
            float curlIntensity = length(curl); // Use curl magnitude to create swirls
            vec3 curlColor = vec3(0.05, 0.02, 0) * curlIntensity * 0.5; // Modulated color based on curl

            
            color = mix(mountainColor,
                        snowColor + curlColor,
                        t);
            //color = snowColor + curlColor;
        }

        // Optional: blend between two textures at boundary
        // For example, to blend from grass to rock:
        //   
        //   color = mix(texture(u_grassTex, uv).rgb,
        //               texture(u_rockTex, uv).rgb,
        //               t);

        // Mixing can be done:
        // float factor = clamp((normalizedHeight - 0.4) / 0.2, 0.0, 1.0);

        // vec3 sandC = texture(u_sandTex, uv).rgb;
        // vec3 grassC = texture(u_grassTex, uv).rgb;

        // terrainColor = mix(sandC, grassC, factor);


        out_Color = vec4(color, 1.0);
        