/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/** Represents the current executable's process.

    This contains methods for controlling the current application at the
    process-level.

    @see Thread, JUCEApplicationBase

    @tags{Core}
*/
class JUCE_API  Process
{
public:
    //==============================================================================
    enum ProcessPriority
    {
        LowPriority         = 0,
        NormalPriority      = 1,
        HighPriority        = 2,
        RealtimePriority    = 3
    };

    /** Changes the current process's priority.

        @param priority     the process priority, where
                            0=low, 1=normal, 2=high, 3=realtime
    */
    static void JUCE_CALLTYPE setPriority (ProcessPriority priority);

    /** Kills the current process immediately.

        This is an emergency process terminator that kills the application
        immediately - it's intended only for use only when something goes
        horribly wrong.

        @see JUCEApplicationBase::quit
    */
    static void JUCE_CALLTYPE terminate();

    //==============================================================================
    /** Returns true if this application process is the one that the user is
        currently using.
    */
    static bool JUCE_CALLTYPE isForegroundProcess();

    /** Attempts to make the current process the active one.
        (This is not possible on some platforms).
    */
    static void JUCE_CALLTYPE makeForegroundProcess();

    /** Hides the application (on an OS that supports this, e.g. OSX, iOS, Android) */
    static void JUCE_CALLTYPE hide();

    //==============================================================================
    /** Raises the current process's privilege level.

        Does nothing if this isn't supported by the current OS, or if process
        privilege level is fixed.
    */
    static void JUCE_CALLTYPE raisePrivilege();

    /** Lowers the current process's privilege level.

        Does nothing if this isn't supported by the current OS, or if process
        privilege level is fixed.
    */
    static void JUCE_CALLTYPE lowerPrivilege();

    //==============================================================================
    /** Returns true if this process is being hosted by a debugger. */
    static bool JUCE_CALLTYPE isRunningUnderDebugger() noexcept;


    //==============================================================================
    /** Tries to launch the OS's default reader application for a given file or URL. */
    static bool JUCE_CALLTYPE openDocument (const String& documentURL, const String& parameters);

    /** Tries to launch the OS's default email application to let the user create a message. */
    static bool JUCE_CALLTYPE openEmailWithAttachments (const String& targetEmailAddress,
                                                        const String& emailSubject,
                                                        const String& bodyText,
                                                        const StringArray& filesToAttach);

    //==============================================================================
   #if JUCE_WINDOWS || DOXYGEN
    /** WINDOWS ONLY - This returns the HINSTANCE of the current module.

        The return type is a void* to avoid being dependent on windows.h - just cast
        it to a HINSTANCE to use it.

        In a normal JUCE application, this will be automatically set to the module
        handle of the executable.

        If you've built a DLL and plan to use any JUCE messaging or windowing classes,
        you'll need to make sure you call the setCurrentModuleInstanceHandle()
        to provide the correct module handle in your DllMain() function, because
        the system relies on the correct instance handle when opening windows.
    */
    static void* JUCE_CALLTYPE getCurrentModuleInstanceHandle() noexcept;

    /** WINDOWS ONLY - Sets a new module handle to be used by the library.

        The parameter type is a void* to avoid being dependent on windows.h, but it actually
        expects a HINSTANCE value.

        @see getCurrentModuleInstanceHandle()
    */
    static void JUCE_CALLTYPE setCurrentModuleInstanceHandle (void* newHandle) noexcept;
   #endif

    //==============================================================================
   #if JUCE_MAC || DOXYGEN
    /** OSX ONLY - Shows or hides the OSX dock icon for this app. */
    static void setDockIconVisible (bool isVisible);
   #endif

    //==============================================================================
   #if JUCE_MAC || JUCE_LINUX || JUCE_BSD || DOXYGEN
    /** UNIX ONLY - Attempts to use setrlimit to change the maximum number of file
        handles that the app can open. Pass 0 or less as the parameter to mean
        'infinite'. Returns true if it succeeds.
    */
    static bool setMaxNumberOfFileHandles (int maxNumberOfFiles) noexcept;
   #endif

private:
    Process();
    JUCE_DECLARE_NON_COPYABLE (Process)
};

} // namespace juce
