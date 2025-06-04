package com.example.cdac_tee

import android.os.Bundle
import com.google.android.material.snackbar.Snackbar
import androidx.appcompat.app.AppCompatActivity
import androidx.navigation.findNavController
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import android.view.Menu
import android.view.MenuItem
import com.example.cdac_tee.databinding.ActivityMainBinding
import android.os.Build
import android.widget.TextView
import android.widget.ScrollView
import java.io.BufferedReader
import java.io.File
import java.io.InputStreamReader

class MainActivity : AppCompatActivity() {
//    override fun onCreate(savedInstanceState: Bundle?) {
//        super.onCreate(savedInstanceState)
//        val textView = TextView(this)
//        textView.text = getSystemInfo()
//        setContentView(textView)
//    }
override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)

    val textView = TextView(this)
    textView.text = getSystemInfo()
    textView.setPadding(16, 16, 16, 16)

    val scrollView = ScrollView(this)
    scrollView.addView(textView)

    setContentView(scrollView)
}


    private fun getSystemInfo(): String {
        val info = StringBuilder()

        info.append("Device Model: ${Build.MODEL}\n")
        info.append("Manufacturer: ${Build.MANUFACTURER}\n")
        info.append("Board: ${Build.BOARD}\n")
        info.append("Brand: ${Build.BRAND}\n")
        info.append("Hardware: ${Build.HARDWARE}\n")
        info.append("Bootloader: ${Build.BOOTLOADER}\n")
        info.append("Build ID: ${Build.ID}\n")
        info.append("Android Version: ${Build.VERSION.RELEASE}\n")
        info.append("SDK Version: ${Build.VERSION.SDK_INT}\n")
        info.append("Kernel Version: ${getCommandOutput("uname -a")}\n")
        info.append("CPU Info:\n${getCommandOutput("cat /proc/cpuinfo")}\n")
        info.append("Memory Info:\n${getCommandOutput("cat /proc/meminfo")}\n")
        info.append("TEE Kernel Logs:\n${getCommandOutput("dmesg | grep -i tee")}\n")
        info.append("TEE Devices:\n")
        info.append("${getCommandOutput("ls /dev/tee0")}\n")
        info.append("${getCommandOutput("ls /dev/teepriv0")}\n")
        val tee0 = File("/dev/tee0")
        info.append("TEE0 Exists: ${tee0.exists()}, Readable: ${tee0.canRead()}, Writable: ${tee0.canWrite()}\n")




        return info.toString()
    }

    private fun getCommandOutput(command: String): String {
        return try {
            val process = ProcessBuilder(*command.split(" ").toTypedArray()).start()
            val reader = BufferedReader(InputStreamReader(process.inputStream))
            val output = StringBuilder()
            var line: String?
            while (reader.readLine().also { line = it } != null) {
                output.append(line).append("\n")
            }
            reader.close()
            output.toString()
        } catch (e: Exception) {
            "Error: ${e.message}\n"
        }
    }
}
