<template>
  <div class="edit">
    <header>
      <van-nav-bar title="" left-arrow @click-left="onClickLeft"></van-nav-bar>
    </header>
    <div class="content">
      <div class="title">填写播放信息</div>
      <van-form
        input-align="right"
        error-message-align="right"
        @submit="onSubmit"
        ref="formRef"
      >
        <van-cell-group inset>
          <van-field
            v-model="deviceId"
            name="deviceId"
            label="deviceId"
            placeholder="请输入deviceId"
            :rules="[{ required: true, message: '请填写设备ID' }]"
          />
          <van-field
            v-model="channelId"
            name="channelId"
            label="channelId"
            placeholder="请输入channelId"
            :rules="[{ required: true, message: '请填写通道ID' }]"
          />
          <van-field
            v-model="kitToken"
            name="kitToken"
            label="kitToken"
            placeholder="请输入kitToken"
            :rules="[{ required: true, message: '请填写kitToken' }]"
          />
        </van-cell-group>
        <van-cell-group inset style="margin-top: 12px">
          <van-field
            v-model="streamIdLabel"
            is-link
            readonly
            name="picker"
            label="清晰度"
            placeholder="点击选择清晰度"
            @click="handlePicker('streamId')"
          />
          <van-field
            v-model="typeLabel"
            is-link
            readonly
            name="picker"
            label="播放类型"
            placeholder="点击选择播放类型"
            @click="handlePicker('type')"
          />
          <van-field
            v-if="type === '2'"
            v-model="recordTypeLabel"
            is-link
            readonly
            name="picker"
            label="录像类型"
            placeholder="点击选择录像类型"
            @click="handlePicker('recordType')"
          />

          <van-field
            v-model="code"
            name="code"
            label="加密秘钥"
            placeholder="请输入加密秘钥"
          />
        </van-cell-group>
      </van-form>

      <footer>
        <div class="footer-wrapper">
          <van-button round color="#F18D00" @click="handleSubmit"
            >生成画面</van-button
          >
        </div>
      </footer>

      <van-popup
        :show="showPicker"
        @update:show="showPicker = $event"
        destroy-on-close
        position="bottom"
      >
        <van-picker
          :columns="columns"
          :model-value="pickerValue"
          @confirm="onPickerConfirm"
          @cancel="showPicker = false"
        />
      </van-popup>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from "vue";
import router from "../router";
import { useRoute } from "vue-router";
const route = useRoute();
const id = ref(route.query.id);
const deviceId = ref("");
const channelId = ref("");
const kitToken = ref("");
const code = ref("");
const streamId = ref("0");
const streamIdLabel = ref("高清");
const type = ref("1");
const typeLabel = ref("实时预览");
const recordType = ref("cloud");
const recordTypeLabel = ref("云录像");
const columns = ref<any[]>([]);
const showPicker = ref(false);
const pickerValue = ref<any[]>([]);
const pickerType = ref("");
const formRef = ref<any>("formRef");
const list = ref<any[]>([]);
onMounted(() => {
  list.value = JSON.parse(localStorage.getItem("list") || "[]");
  if (id.value) {
    const info = list.value.find((item: any) => item.id == id.value);
    if (info) {
      deviceId.value = info.deviceId;
      channelId.value = info.channelId;
      kitToken.value = info.kitToken;
      code.value = info.code;
      streamId.value = info.streamId;
      streamIdLabel.value =
        streamIdOptions.find((item) => item.value == info.streamId)?.text ||
        "高清";
      type.value = info.type;
      typeLabel.value =
        typeOptions.find((item) => item.value == info.type)?.text || "实时预览";
      recordType.value = info.recordType;
      recordTypeLabel.value =
        recordTypeOptions.find((item) => item.value == info.recordType)?.text ||
        "云录像";
    }
  }
});

const streamIdOptions = [
  { text: "高清", value: "0" },
  { text: "标清", value: "1" },
];

const typeOptions = [
  { text: "实时预览", value: "1" },
  { text: "录像回放", value: "2" },
];

const recordTypeOptions = [
  { text: "云录像", value: "cloud" },
  { text: "本地录像", value: "localRecord" },
];
const onClickLeft = () => {
  router.back();
};
const handlePicker = (type: string) => {
  switch (type) {
    case "streamId":
      columns.value = streamIdOptions;
      break;
    case "type":
      columns.value = typeOptions;
      break;
    case "recordType":
      columns.value = recordTypeOptions;
      break;
    default:
      break;
  }
  pickerType.value = type;
  showPicker.value = true;
};

const onPickerConfirm = ({
  selectedValues,
  selectedOptions,
}: {
  selectedValues: any[];
  selectedOptions: any[];
}) => {
  switch (pickerType.value) {
    case "streamId":
      streamIdLabel.value = selectedOptions[0]?.text;
      streamId.value = selectedValues[0];
      break;
    case "type":
      typeLabel.value = selectedOptions[0]?.text;
      type.value = selectedValues[0];
      break;
    case "recordType":
      recordTypeLabel.value = selectedOptions[0]?.text;
      recordType.value = selectedValues[0];
      break;
    default:
      break;
  }
  pickerValue.value = selectedValues;
  showPicker.value = false;
};

const handleSubmit = () => {
  formRef.value.submit();
};

const onSubmit = () => {
  const info: any = {
    deviceId: deviceId.value,
    channelId: channelId.value,
    kitToken: kitToken.value,
    code: code.value,
    streamId: streamId.value,
    type: type.value,
    recordType: recordType.value,
  };
  let newList: any[] = [];
  if (id.value) {
    info.id = id.value;
    list.value = list.value.map((item) => {
      if (item.id == id.value) {
        return info;
      }
      return item;
    });
  } else {
    info.id = String(Date.now());
    newList = [...list.value, info];
  }
  localStorage.setItem(
    "list",
    JSON.stringify(newList.length ? newList : list.value)
  );
  router.back();
};
</script>

<style scoped lang="less">
.edit {
  width: 100%;
  min-height: 100vh;
  background-color: #f6f6f6;
  display: flex;
  flex-direction: column;
}

.van-nav-bar {
  background: transparent;
}
.van-hairline--bottom:after {
  border: none;
}

.content {
  flex: 1;
  padding-bottom: 120px;
  .title {
    padding: 20px 25px;
    font-weight: 800;
    font-size: 23px;
    color: #2c2c2c;
    line-height: 32px;
  }
}

footer {
  position: fixed;
  left: 0;
  bottom: 0;
  overflow: hidden;
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  .footer-wrapper {
    border-radius: 10px 10px 0px 0px;
    background: #ffffff;
    width: 100%;
    padding: 20px 35px 40px;
  }
  button {
    width: 100%;
    min-height: 40px;
  }
}

/deep/.van-nav-bar .van-icon {
  color: #2c2c2c !important;
}
.van-cell {
  padding: 20px;
}
</style>
