<template>
  <div class="page">
    <header>
      <div class="title-desc">
        <h1>轻应用H5体验</h1>
        <span>点击"添加"画面，即可体验</span>
      </div>
    </header>

    <div class="main">
      <div class="main-wrap">
        <div class="empty" v-if="list.length === 0">
          <img src="../assets/images/empty.png" alt="empty" />
          <span>暂无画面</span>
          <van-button
            plain
            round
            color="#F18D00"
            size="small"
            @click="handleAdd"
            >添加</van-button
          >
        </div>

        <template v-else>
          <div class="list">
            <div class="add" @click="handleAdd">
              <img src="../assets/images/add.png" alt="add" />
              <span>添加画面</span>
            </div>
            <ul>
              <li v-for="(item, index) in list" :key="item.url">
                <van-swipe-cell>
                  <div class="item-wrap">
                    <div @click="handleCheck(item.id)">
                      <img
                        v-if="checkedId === item.id"
                        src="../assets/images/check.png"
                        alt="check"
                      />
                      <img
                        v-else
                        src="../assets/images/check_n.png"
                        alt="uncheck"
                      />
                    </div>

                    <img src="../assets/images/icon_huamian.png" alt="check" />
                    <div class="item-info" @click="handleEdit(item.id)">
                      <span>画面{{ index + 1 }}</span>
                      <span>{{
                        `/${item.deviceId}/${item.channelId}/${item.type}?streamId=${item.streamId}&code=${item.code}&kitToken=${item.kitToken}`
                      }}</span>
                    </div>
                  </div>
                  <template #right>
                    <van-button
                      square
                      text="删除"
                      type="danger"
                      class="delete-button"
                      @click="handleDelete(item.id)"
                    />
                  </template>
                </van-swipe-cell>
              </li>
            </ul>
          </div>
          <footer>
            <div class="footer-wrapper">
              <van-button
                round
                color="#F18D00"
                :disabled="!checkedId"
                @click="handlePlay"
                >立即播放</van-button
              >
            </div>
          </footer>
        </template>
      </div>
    </div>

    <div class="logview">
      <van-button size="small" type="success" @click="handleStartLog"
        >开启缓存日志</van-button
      >
      <van-button size="small" type="success" @click="handleEndLog"
        >结束缓存日志</van-button
      >
    </div>
  </div>
</template>

<script setup lang="ts">
import { showConfirmDialog } from "vant";
import { onMounted, ref } from "vue";
import router from "../router";

const list = ref<any[]>([]);
const checkedId = ref("");

onMounted(() => {
  list.value = JSON.parse(localStorage.getItem("list") || "[]");
});

const handleCheck = (id: string | number) => {
  checkedId.value = id.toString();
};

const handlePlay = () => {
  router.push({
    path: "/player",
    query: {
      id: checkedId.value,
    },
  });
};

const handleAdd = () => {
  router.push({
    path: "/edit",
  });
};

const handleEdit = (id: string | number) => {
  router.push({
    path: "/edit",
    query: {
      id: id.toString(),
    },
  });
};

const handleDelete = (id: string | number) => {
  showConfirmDialog({
    title: "注意",
    message: "确定删除此画面？",
  })
    .then(() => {
      list.value = list.value.filter((item) => item.id != id);
      localStorage.setItem("list", JSON.stringify(list.value));
    })
    .catch(() => {
      // on cancel
    });
};

const handleStartLog = () => {
  console.log("开启缓存日志");
  window.imouPlayer.debugSaveLogFile(1);
};

const handleEndLog = () => {
  console.log("结束缓存日志");
  window.imouPlayer.debugSaveLogFile(0);
};
</script>

<style lang="less" scoped>
.page {
  height: 100vh;
  overflow: hidden;
  background: #f6f6f6;
  display: flex;
  flex-direction: column;
}

header {
  background: url("../assets/images/bg.png") no-repeat center -44px;
  background-size: 100%;
  height: 160px;

  .edit-text {
    font-weight: 500;
    font-size: 14px;
    color: #2c2c2c;
  }

  .van-nav-bar {
    background: transparent;
  }
  .van-hairline--bottom:after {
    border: none;
  }

  .title-desc {
    padding: 52px 25px;

    h1 {
      font-weight: 800;
      font-size: 23px;
      color: #2c2c2c;
      line-height: 32px;
      margin-bottom: 12px;
    }

    span {
      font-weight: 500;
      font-size: 14px;
      color: #8f8f8f;
    }
  }
}

.main {
  flex: 1;
  padding: 0 12px;
  overflow-y: auto;
  .main-wrap {
    display: flex;
    flex-direction: column;
    height: 100%;
  }
}

.empty {
  background: #fff;
  border-radius: 10px 10px 0px 0px;
  height: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  img {
    width: 150px;
    height: 150px;
  }
  span {
    font-weight: 500;
    font-size: 14px;
    color: #8f8f8f;
    margin-bottom: 20px;
  }
  button {
    min-width: 120px;
    min-height: 40px;
  }
}

.add {
  display: flex;
  align-items: center;
  padding: 15px 12px;
  background: #ffffff;
  border-radius: 10px;
  img {
    width: 35px;
    height: 35px;
  }
  span {
    font-weight: 500;
    font-size: 16px;
    color: #f18d00;
    margin-left: 10px;
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

.list {
  min-height: 100%;
  flex: 1;
  display: flex;
  flex-direction: column;

  ul {
    border-radius: 10px;
    flex: 1;
    margin-top: 12px;
    overflow-x: hidden;
    margin-bottom: 116px;

    li {
      background: #ffffff;
      border-bottom: 1px solid #f6f6f6;

      &:last-child {
        border-radius: 0 0 10px 10px;
        border: 0;
      }

      .item-wrap {
        display: flex;
        align-items: center;
        height: 91px;
        padding: 0 12px;
        img:first-child {
          width: 18px;
          height: 18px;
        }
        img:nth-child(2) {
          width: 24px;
          height: 24px;
          margin: 0 12px;
        }
        .item-info {
          flex: 1;
          display: flex;
          flex-direction: column;
          justify-content: center;
          span:first-child {
            font-weight: 500;
            font-size: 14px;
            color: #2c2c2c;
          }
          span:last-child {
            font-weight: 500;
            font-size: 12px;
            color: #8f8f8f;
            line-height: 18px;
            word-break: break-word;
          }
        }
      }
      .delete-button {
        height: 100%;
      }
    }
  }
}

.logview {
  position: fixed;
  top: 20px;
  left: 20px;
  display: flex;
  gap: 10px;
}
</style>
