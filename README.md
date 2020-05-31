[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.md)

See the [original GitHub page](https://github.com/baldurk/renderdoc) for more details on RenderDoc
including instructions for building.

This is a fork of RenderDoc with a selection of patches applied to enable building RenderDoc.dll for use with UWP applications, including on the ARM and ARM64 devices. The motivating force was to get RenderDoc graphics debugging, beloved by many, working for HoloLens 2 applications.

# Instructions

The process of using RenderDoc with a UWP application is not yet as straightforward as it is for desktop applications. The following steps are required:
1. Clone this repo and build the "DLL\RenderDoc" project (not the entire solution) in Visual Studio for the architecture of your UWP application. This will produce renderdoc.dll.
2. Include your built copy of RenderDoc.dll in your Appx/MSIX package. If you use a standard .vcxproj to package your application, you can add the following into an `ItemGroup`:
    ```xml
    <None Include="[Path to RenderDoc git repository]\$(Platform)\Release\renderdoc.dll">
      <DeploymentContent>true</DeploymentContent>
    </None>
    ```
    If your UWP project is not built through Visual Studio, you will need to augment the packaging system to include renderdoc.dll.
3. Modify the AppxManifest.xml of your UWP application file to include two additional capabilities:
    ```xml
      <Capabilities>
        <Capability Name="internetClientServer" />
        <Capability Name="privateNetworkClientServer"/>
        [...]
      </Capabilities>
    ```

4. Early in your application start up, load RenderDoc.dll:
    ```c
    HMODULE renderDocModule = ::LoadPackagedLibrary(L"renderdoc.dll", 0);
    ```
    RenderDoc.dll will initialize itself when it is loaded, starting a TCP server and hooking the various graphics APIs.

    I recommend debugging your application and checking the value of `renderDocModule` to ensure it is not NULL which would indicate the module failed to load.

5. (Local debugging only) If you are debugging a UWP application on the same machine as the RenderDoc GUI, you will need to disable loopback server isolation by running the following command from an elevated command prompt:
    ```
    CheckNetIsolation loopbackexempt -is -n=<Package Family Name>
    ```
    You can get your Package Family Name a number of different ways:
    * Open your Package.appxmanifest file in the Visual Studio "App Manifest Designer" (this is the default handler for AppxManifest files in Visual Studio). Go to the "Packaging" tab and the Package family name will be one of the read-only fields.
    * Assuming the application is already installed on your PC, open PowerShell and run "Get-AppxPackage \<name\>" where `<name>` is the value from the AppxManifest's `Identity` `Name` attribute.
    * Read the value out by calling Windows::ApplicationModel::Package::Current().Id().[FamilyName()](https://docs.microsoft.com/en-us/uwp/api/windows.applicationmodel.packageid.familyname) inside your application.

6. Deploy and launch your application with the above modifications.
7. Open the RenderDoc GUI and go to File and select "Attach To Running Instance". If your application is running on a different device, add the IP address of the device. You can get the IP from the Windows Device Portal's Networking page (e.g. 192.168.1.123) on the HoloLens.

At this point if everything has gone well, you should see your UWP application running and can attach to it.

**Warning**: If you are debugging [Windows Mixed Reality applications](https://docs.microsoft.com/en-us/windows/mixed-reality/directx-development-overview)
then you may not see an option to capture a frame. This is because WMR applications do not use the DXGI swapchain "Present" call which is hooked by RenderDoc. Instead you will need to use the [RenderDoc API](https://renderdoc.org/docs/in_application_api.html)  to call `StartFrameCapture` and `EndFrameCapture`. If your application uses OpenXR, this is **NOT** necessary and the Microsoft OpenXR runtime will ensure that RenderDoc can capture the provided projection layer XrSwapchain images.

# Background Information

Developers usually use the RenderDoc GUI to launch their application for graphics debugging. This will inject renderdoc.dll into the process and allow it to hook the API calls. renderdoc.dll will start a TCP server to listen for incoming connections. There are a set of problems that need to be overcome to enable debugging for UWP applications, especially on ARM/ARM64 architectures.

#### Problem 1
RenderDoc.dll uses the dynamic CRT. This CRT is not available when loaded inside of UWP applications. This will manifest as the DLL failing to load due to missing dependencies.

**Solution**:  Change the compiler to use the static CRT. While the static CRT is not intended (?) for use in UWP applications, it introduces no dependencies on disallowed APIs and works fine.

#### Problem 2
RenderDoc.dll is only buildable for X64/X86 and thus won't work on ARM/ARM64-based Windows devices.

**Solution**:  A number of changes were made to the code to fix the build on ARM/ARM64. Most importantly, the Windows 8.1 SDK was used which does not support ARM64 and so the projects were upgraded to the Windows 10 SDK. Secondly, the v140 platform toolset does not support targetting ARM/ARM64 for Win32 build targets and so the projects were upgraded to use platform toolset v142.

#### Problem 3
RenderDoc.dll listens for incoming connections from the RenderDoc GUI which by default is blocked in a UWP app container.

**Solution**: Two capabilities, internetClientServer and privateNetworkClientServer, must be added to the manifest to allow incoming connections from other devices on the network.

#### Problem 4 (local debugging only)
The UWP app container blocks all network connections that come from the local machine (for example, if you wanted to debug a UWP application on your development PC). There is no capability that will unblock local connections to a TCP server in a UWP application. This is called "loopback server isolation" and it is intended to prevent UWP applications from breaking out of the app container and talking to more privileged applications on the PC.

**Solution**: The command-line tool CheckNetIsolation.exe that comes with Windows can be used to disable loopback server isolation. ```CheckNetIsolation loopbackexempt -is -n=<Package Family Name>```

#### Problem 5
RenderDoc.dll must be loaded early into a UWP application and Windows provides no built-in tool to inject a DLL into a process on startup.

**Solution**: You must package RenderDoc.dll in with your application and load it using [LoadPackagedLibrary](https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-loadpackagedlibrary) before the graphics API is used. In the future I believe a tool can be written to automate this, at least for local debugging, using [plmdebug.exe](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/plmdebug) or the underlying PLM APIs.
