package com.mingyuans.smoke.android;
/*****************************************************************************
 Copyright mingyuans

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ******************************************************************************/

import android.util.Log;

import com.mingyuans.smoke.CollectionUtil;
import com.mingyuans.smoke.ConsolePrinter;
import com.mingyuans.smoke.Smoke;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by yanxq on 17/3/15.
 */

public class AndroidConsolePrinter extends Smoke.Process {
    private static final int MAX_LINE_LENGTH = 3990;

    private boolean consoleEnable = true;

    @Override
    public boolean proceed(Smoke.LogBean logBean, List<String> messages, Chain chain) {
        if (CollectionUtil.isEmpty(messages) && !consoleEnable) {
            return true;
        }

        int currentBuilderLength = 0;
        LinkedList<StringBuilder> builders = new LinkedList<>();
        builders.addLast(new StringBuilder());
        for (String line : messages) {
            if (line.length() > MAX_LINE_LENGTH) {
                int splits = line.length() / MAX_LINE_LENGTH + 1;
                for (int i = 0, startIndex = 0; i < splits; i++) {
                    int endIndex = startIndex + MAX_LINE_LENGTH > line.length()?
                            line.length(): startIndex + MAX_LINE_LENGTH;
                    String lineText = line.substring(startIndex,endIndex);
                    currentBuilderLength = appendString(builders,currentBuilderLength,lineText);
                    startIndex = endIndex;
                }
            } else {
                currentBuilderLength = appendString(builders,currentBuilderLength,line);
            }
        }

        for (StringBuilder builder : builders) {
            Log.println(logBean.level, logBean.tag, builder.toString());
        }
        return chain.proceed(logBean,messages);
    }

    private int appendString(LinkedList<StringBuilder> builders,int builderLength,String line) {
        int lineLength = line.length();
        if (builderLength + lineLength >= MAX_LINE_LENGTH) {
            builders.addLast(new StringBuilder());
            builderLength = 0;
        }
        builders.getLast().append(line.endsWith("\n")? line : line + "\n");
        builderLength += lineLength;
        return builderLength;
    }

    @Override
    public boolean notification(String event, Object value) {
        if (ConsolePrinter.CONSOLE_ENABLE_B.equals(event)) {
            consoleEnable = (boolean) value;
            return true;
        }
        return super.notification(event, value);
    }
}
